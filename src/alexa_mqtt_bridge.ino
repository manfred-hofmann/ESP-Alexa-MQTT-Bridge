// ******************************************************************************
// ALEXA MQTT BRIDGE
//
// @mc ESP8266 Version 2.6.3
// @created 12.07.2021
//
// This source file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This source file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ******************************************************************************
// ******************************************************************************

// Einstellungen für das Board: (LOLIN(WEMOS) D1 R2 & mini)
// CPU Frequenz auf 160 MHz
// Flash Size 4MB ( FS:none OTA~1019KB)
// SLL Support Basic

// Im Normalbetrieb immer alle DEBUG Schalter aus.

// folgende Libraries werden benötigt:

//Espalexa:
//Open your Arduino IDE and go to Sketch > Include Library > Manage Libraries. The Library Manager should open.
//Search for "Espalexa" on the Search box and install the Espalexa library by Christian Schwinne. --> Espalexa.h (Version 2.7.0)

//MQTT
//Open your Arduino IDE and go to Sketch > Include Library > Manage Libraries. The Library Manager should open.
//Search for "PubSubClient" on the Search box and install the MQTT library by Nick O'Leary. --> PubSubClient.h (Version 2.8.0)



#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <TimeLib.h>

#include "Configuration.h"
#include <Espalexa.h>
#include <PubSubClient.h>

#include "Settings.h"
#include "icons.h"


#define VERSION 20210809


//callback functions
void handle_alexa(EspalexaDevice* dev);

//create devices yourself
EspalexaDevice* alexa[ESPALEXA_MAXDEVICES];

Espalexa espalexa;

s_mysettings mysetting;

// WebServer
ESP8266WebServer webServer(80);

// DNS server 
const byte DNS_PORT = 53; 
DNSServer dnsServer; 

// WiFi
IPAddress myIP;
String STA_ssid;
String STA_pass;

// MQTT
WiFiClient espClient;
PubSubClient mqttclient(espClient);

//Variablen
time_t upTime = 0;
unsigned long aktmillis = 0;
unsigned long mqttrcmillis = 0;
bool wifimode = false;  // AP-Mode = false STA-Mode = true
bool mqtt_ready = false;
bool mqtt_ready_out = false;
int alexa_idx = 0;
int ap_sekunden_loop = 0;
int sta_sekunden_loop = 0;
int mqttloop = 0;
uint8_t alexa_idxpoint[ESPALEXA_MAXDEVICES+1] = {};

//######################################## SETUP ######################################## 
void setup() {

  Serial.begin(115200);
  delay(500); // Kurze Pause, damit wir Zeit haben den Seriellen Monitor zu öffnen.
#ifdef PIN_LED
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
#endif

  loadFromEEPROM();

//  WiFi.disconnect(true);
  WiFi.hostname(String(mysetting.systemname));
  WiFi.mode(WIFI_STA);
  delay (1000);
  myIP = WiFi.localIP();
  STA_ssid = WiFi.SSID();
  STA_pass = WiFi.psk();
#ifdef DEBUG_WLAN
  Serial.print("Hostname: ");
  Serial.println(String(mysetting.systemname));
  Serial.print("STA IP address: ");
  Serial.println(myIP);
  Serial.print("SSID: ");
  Serial.println(STA_ssid);
#endif

  
  int versuche = 0;
  while (WiFi.status() != WL_CONNECTED  && versuche < WIFI_SETUP_TIMEOUT * 2 ) {
    delay(400);
// Flash ESP LED
#ifdef PIN_LED
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
#endif
#ifdef DEBUG_WLAN
    Serial.print(".");
    if ( (versuche+1) %50 == 0 ) Serial.println(".");
#endif
delay(100);
#ifdef PIN_LED
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
#endif
    versuche++;
  }
  Serial.println(".");
  WiFi.setAutoReconnect(true);

  if (!WiFi.isConnected())
  {

    WiFi.softAP(String(mysetting.systemname));
#ifdef DEBUG_WLAN
    Serial.println("No WLAN connected. Staying in AP mode.");
#endif
#ifdef PIN_LED
    digitalWrite(PIN_LED, HIGH);
#endif
    delay(3000);
    WiFi.mode(WIFI_AP);
    wifimode = false;
    myIP = WiFi.softAPIP();
#ifdef DEBUG_WLAN
    Serial.println("AP IP address: ");
    Serial.println(myIP);
#endif
    // Starte DNS Server für captive portal
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError); 
    dnsServer.start(DNS_PORT, "*", myIP);
    
  }
  else
  {
#ifdef DEBUG_WLAN
    Serial.println("WLAN connected. Switching to STA mode.");
#endif
#ifdef PIN_LED
    digitalWrite(PIN_LED, LOW);
#endif
    WiFi.mode(WIFI_STA);
    wifimode = true;
    delay(1000);
    myIP = WiFi.localIP();
    STA_ssid = WiFi.SSID();
    STA_pass = WiFi.psk();
#ifdef DEBUG_WLAN
    Serial.print("Hostname: ");
    Serial.println(String(mysetting.systemname));
    Serial.print("STA IP address: ");
    Serial.println(myIP);
    Serial.print("SSID: ");
    Serial.println(STA_ssid);
#endif
//    Serial.print("Passwort: ");
//    Serial.println(STA_pass);    


    
// ########################## ALEXA #########################

#ifdef DEBUG_ALEXA
    Serial.println("Start ALEXA");
#endif
      alexa_idx = 0;
      for (int a = 0; a <= ESPALEXA_MAXDEVICES; a++) 
      {
        if ( mysetting.alexa[a].aktiv )
        {
          alexa[alexa_idx] = new EspalexaDevice(String(mysetting.alexa[a].text), handle_alexa, EspalexaDeviceType::dimmable);
          if ( espalexa.addDevice(alexa[alexa_idx]) ) 
          { 
            alexa_idxpoint[alexa_idx] = a;
            
#ifdef DEBUG_ALEXA
            Serial.printf("Alexa %i als %s angelegt.\n",alexa_idx,mysetting.alexa[a].text);
#endif
            alexa_idx++;
          }
          else
          {
            delay (500);
#ifdef DEBUG_ALEXA
            Serial.printf("Alexa %i als %s nicht angelegt!!!\n",alexa_idx,mysetting.alexa[a].text);
#endif
          }
        }
      }

    
#ifdef DEBUG
    Serial.println("Start mDNS responder.");
#endif
    // mDNS is needed to see HOSTNAME in Arduino IDE
    MDNS.begin(String(mysetting.systemname));

#ifdef DEBUG
    Serial.println("Start OTA service.");

    ArduinoOTA.onStart([]()
      {
        Serial.println("Start OTA update.");
      });
    ArduinoOTA.onError([](ota_error_t error)
      {
        Serial.println("OTA Error: " + String(error));
        if (error == OTA_AUTH_ERROR) Serial.println("Auth failed.");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin failed.");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect failed.");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive failed.");
        else if (error == OTA_END_ERROR) Serial.println("End failed.");
      });
    ArduinoOTA.onEnd([]()
      {
        Serial.println("End OTA update.");
      });
#endif
#ifdef OTA_PASS
    ArduinoOTA.setPassword(OTA_PASS);
#endif
    ArduinoOTA.begin();

//MQTT
#ifdef DEBUG
  Serial.println("Start MQTT.");
#endif
  mqttclient.setServer(mysetting.mqttbroker, 1883);
  mqttclient.setCallback(mqttcallback);
  delay(2000);
  mqttreconnect();
  }

#ifdef DEBUG
  Serial.println("Start WEBSERVER.");
#endif

  espalexa.begin(&webServer);
  setupWebServer();
  
}


//#####################################################################
// START L O O P
//#####################################################################

void loop() {
  dnsServer.processNextRequest();
  MDNS.update();
  espalexa.loop();
  ArduinoOTA.handle();
  if ( wifimode )
  {
    if (!mqttclient.connected() && millis() > mqttrcmillis + 30000 ) 
    {
      mqttreconnect();
      mqttrcmillis = millis();
    }
    mqttclient.loop();
  }
  
// Jede Sekunde:
  if ( millis() > aktmillis + 1000 ) 
  {
    if ( mqttloop > 43200 ) 
    {
      mqttloop = 0;
      mqttclient.disconnect();
      delay(1000);
#ifdef DEBUG
      Serial.println("12h Timeout: MQTT-Reconnect");
#endif
      mqttreconnect();
      
      delay(1000);
      if (!mqttclient.connected() ) 
      {
#ifdef DEBUG
      Serial.println("Fehler MQTT-Reconnect. Reboot...");
#endif
        delay (4000);
        ESP.restart();
      }
      else
      {
#ifdef DEBUG
      Serial.println("MQTT-Reconnect- OK!");
#endif       
      }
    }
    
    upTime = millis()/1000;
    mqttloop++;
#ifdef PIN_LED
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
#endif
    if ( !wifimode )      //AP-Mode
    {
      ap_sekunden_loop++;
      if ( ap_sekunden_loop > AP_WIFI_TIMEOUT )
      {
#ifdef DEBUG_WLAN
        Serial.println("AP WLAN TimeOut. Reboot!");
#endif
        delay(5000);
        ESP.restart();
      }
    }
    else //STA-Mode
    {
      if (!WiFi.isConnected()) 
      {
        sta_sekunden_loop++;
        if ( sta_sekunden_loop > WIFI_SETUP_TIMEOUT )
        {
#ifdef DEBUG_WLAN
          Serial.println("STA WLAN Lost. Reboot!");
#endif
          delay(5000);
          ESP.restart();
        }
      }
      else
      {
        sta_sekunden_loop = 0;
      }
    }
    aktmillis = millis();
  }
  

  if ( wifimode )
  {
    if (mqttloop > 10 && mqttclient.connected() ) 
    {
      mqtt_ready = true;
    }
    if ( mqtt_ready && !mqtt_ready_out )
    {
      mqtt_ready_out = true;
      Serial.println("MQTT Ready");  
    }
  }
}

//#####################################################################
// END L O O P
//#####################################################################



//################################################################################################################################################
//##### Root
//################################################################################################################################################

void handleRoot()
{

#ifdef DEBUG
  Serial.println ( "get HostHeader:" + webServer.hostHeader());
#endif

  String message = F("<!doctype html>"
    "<html><head>"
    "<title>ESP ALEXA IOBROKER Gateway</title>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<meta charset=\"UTF-8\">"
    "<link rel=\"icon\" type=\"image/png\" sizes=\"192x192\"  href=\"");
    message += FPSTR(favicon192);
    message += F("\">\n"
    "<link rel=\"icon\" type=\"image/png\" sizes=\"32x32\"  href=\"");
    message += FPSTR(favicon32);
    message += F("\">\n"
    "<style>\n"
    "body{background-color:#aaaaaa;text-align:center;color:#333333;font-family:Sans-serif;}\n"
    "button{background-color:#353746;text-align:center;line-height: 22px;color:#FFFFFF;width:150px;height:32px;margin:5px;padding:1px;border:2px solid #FFFFFF;font-size:14px;border-radius:15px;cursor: pointer;}\n"
    "h2 {color: #FFFFFF;text-shadow: 2px 2px 2px Black;}\n"
    "span {color: Black;}\n"
    "input[type=submit]{background-color:#1FA3EC;text-align:center;color:#FFFFFF;width:200px;padding:12px;border:5px solid #FFFFFF;font-size:20px;border-radius:10px;}\n"
    "table{border-collapse:collapse;margin:0px auto;} td{padding:12px;border-bottom:1px solid #ddd;} tr:first-child{border-top:1px solid #ddd;} td:first-child{text-align:right;} td:last-child{text-align:left;}\n"
    "select{font-size:16px;}\n"
    "#ssel{border-top:3px solid #aaaaaa;border-bottom:3px solid #aaaaaa;}"
    "#wt{padding:3px}"
    "</style>"
    "</head>"
    "<body>\n");
    message += F("<h2>"); message += String(mysetting.systemname); message += F(" Settings</h2>");

    message += F("<form name=\"gwupdate\" action=\"/gwupdate\" method=\"POST\">\n"
    "<table>\n");
    
    for (int z = 0; z < ESPALEXA_MAXDEVICES; z++) 
    {
      message += F("<tr><td>");
      message += String(z+1) + ". ";
      message += F("<input type=\"checkbox\" name=\"alexaaktiv_"); 
      message += String(z) + "\" value=\"1\"";
      if ( z == 0) message += " disabled ";
      if ( mysetting.alexa[z].aktiv )
      {
        message += F("checked>\n");
      }  
      else
      {
        message += F(">");
      }
      message += F("</td>\n<td>");
      message += F("<input type=\"text\" value=\""); 
      message += String(mysetting.alexa[z].text);
      message += F("\" name=\"alexatext_");
      message += String(z);
      message += F("\" minlength=\"3\" maxlength=\"30\" size=\"30\">\n");
      message += F("Value: ");
      message += String(mysetting.alexa[z].value);
      message += F("</td></tr>\n");
    }
    
    message += F("</table>\n");
    message += F("<br>");
    message += F("<button title=\"Save Settings.\"><span style=\"color:White;font-size:14px;\">&#128077; speichern</span></button>"
//                 "<button  type=\"submit\" formaction=\"/handlewifisettings\"><span style=\"color:White;font-size:14px;\"&#128295;</span></button>"
                 "<button title=\"wlan\" type=\"button\" onclick=\"window.location.href='/handlewifisettings'\"><span style=\"color:White;font-size:14px;\">&#128295; WiFi/MQTT</span></button>"
      "</form>\n");
      
    message += F("</body></html>");

    webServer.send(200, "text/html", message);
}
//################################################################################################################################################
//##### Settings (WiFi)
//################################################################################################################################################

void handleWiFiSettings()
{
    String cssid;
    String ownssid;
    String STA_pass = WiFi.psk();
#ifdef DEBUG_WLAN
   if ( !wifimode ) Serial.println ( "Enter AP WifiSettings:" + webServer.hostHeader());
#endif
    ap_sekunden_loop = 0;       // Verbindung hergestellt reset der AP-Zeit
    ownssid = WiFi.SSID();
    if ( ownssid == "" ) ownssid = "SID";
    int n = WiFi.scanNetworks();
    int indices[n];
    
    if (n == 0) {
#ifdef DEBUG_WLAN
          Serial.println("no networks found");
#endif
    } else {
     //sort networks

      for (int i = 0; i < n; i++) {
        indices[i] = i;
      }
      // RSSI SORT
      for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
          if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
            std::swap(indices[i], indices[j]);
          }
        }
      }
      // remove duplicates
      for (int i = 0; i < n; i++) {
        if (indices[i] == -1) continue;
        cssid = WiFi.SSID(indices[i]);
        for (int j = i + 1; j < n; j++) {
          if (cssid == WiFi.SSID(indices[j])) {
            indices[j] = -1; // set dup aps to index -1
          }
        }
      }
    }
    
    String message = F("<html><head>"
    "<title>WiFi/MQTT Settings</title>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<meta charset=\"UTF-8\">"
    "<link rel=\"icon\" type=\"image/png\" sizes=\"192x192\"  href=\"");
    message += FPSTR(favicon192);
    message += F("\">\n"
    "<link rel=\"icon\" type=\"image/png\" sizes=\"32x32\"  href=\"");
    message += FPSTR(favicon32);
    message += F("\">\n"
    "<style>\n"
    "body{background-color:#aaaaaa;text-align:center;color:#333333;font-family:Sans-serif;}\n"
    "button{background-color:#353746;text-align:center;line-height: 22px;color:#FFFFFF;width:150px;height:32px;margin:5px;padding:1px;border:2px solid #FFFFFF;font-size:14px;border-radius:15px;cursor: pointer;}\n"
    "h2 {color: #FFFFFF;text-shadow: 2px 2px 2px Black;}\n"
    "span {color: Black;}\n"
    "input[type=submit]{background-color:#1FA3EC;text-align:center;color:#FFFFFF;width:200px;padding:12px;border:5px solid #FFFFFF;font-size:20px;border-radius:10px;}\n"
    "table{border-collapse:collapse;margin:0px auto;} td{padding:12px;border-bottom:1px solid #ddd;} tr:first-child{border-top:1px solid #ddd;} td:first-child{text-align:right;} td:last-child{text-align:left;}\n"
    "select{font-size:16px;}\n"
    "#ssel{border-top:3px solid #aaaaaa;border-bottom:3px solid #aaaaaa;}"
    "#wt{padding:3px}"
    "</style>"
    "</head><body>"
      "<h2>WiFi/MQTT Settings</h2>"
      "<form name=\"wifi\" action=\"/wifiupdate\" method=\"POST\">"
      "<table>"
      "<tr><td width=50>Systemname:</td><td>");
//############ HOSTNAME
      message += F("<input type =\"text\" id=\"systemname\" name=\"systemname\" size=\"29\" minlength=\"5\" maxlength=\"29\" value= \"");
      message += String(mysetting.systemname) + "\">";
      message += F("</td></tr>"
      
      "<tr><td width=50>SID:</td><td>");
  message += "<input type=\"search\" id=\"wlansid\" list=\"wlanliste\" placeholder=\"" + ownssid +"\" name=\"wlansid\">";
  message += F("<datalist id=\"wlanliste\">");
     for (int i = 0; i < n; i++) {
       if (indices[i] == -1) continue; // skip dups
       if ( WiFi.RSSI(indices[i]) < -90 ) continue;  // zu schlechter Empfangspegel
#ifdef DEBUG
       Serial.print(WiFi.SSID(indices[i]));
       Serial.print(" : ");
       Serial.println (WiFi.RSSI(indices[i]));
#endif
       message += "<option value=\"" + WiFi.SSID(indices[i]) +"\">\n";
     }
     message += F("</datalist>");
     message += F("<br><input type=\"password\" value=\"");
     message += STA_pass; 
     message += F("\" id=\"wlanpw\" name=\"wlanpw\" minlength=\"6\" maxlength=\"29\">");
     message += F("<br><input type=\"checkbox\" onclick=\"shwlanpw()\"><small>zeige Passwort</small>");
     message += F("</td></tr>\n");
//############ MQTT     
     message += F("<tr><td width=50>MQTT Server</td><td>");
     message += F("<input type =\"text\" id=\"mqttbroker\" name=\"mqttbroker\" size=\"29\" minlength=\"5\" maxlength=\"49\" value= \"");
     message += String(mysetting.mqttbroker) + "\">";
     message += F("</td></tr>\n");
     message += F("<tr><td width=50><small>MQTT</small> User<br>Passwort</td><td>");
     message += F("<input type =\"text\" id=\"mqttuser\" name=\"mqttuser\" size=\"29\" minlength=\"5\" maxlength=\"29\" value= \"");
     message += String(mysetting.mqttuser) + "\">";
     message += F("<br><input type=\"password\" value=\"");
     message += String(mysetting.mqttpw); 
     message += F("\" id=\"mqttpw\" name=\"mqttpw\" minlength=\"6\" maxlength=\"29\">");
     message += F("<br><input type=\"checkbox\" onclick=\"shmqttpw()\"><small>zeige Passwort</small>");
     message += F("</td></tr>\n");     
//############ MQTT Root   
     message += F("<tr><td width=50><small>MQTT ROOT</small></td><td>");
     message += F("<input type =\"text\" id=\"mqttroot\" name=\"mqttroot\" size=\"29\" minlength=\"2\" maxlength=\"29\" value= \"");
     message += String(mysetting.mqttroot) + "\">"; 
     message += F("</td></tr>\n"
      "</table>"
      "<br>"
      "<button name=\"action\" value=\"1\"><span style=\"color:White;font-size:14px;\">&#128077; speichern</span></button>");
     message += "<br><br>\nUptime: " + String(int(upTime / 86400)) + " Tage, " + String(hour(upTime)) + " Stunden, " + String(minute(upTime)) + " Minuten, " + String(second(upTime)) + " Sekunden";
     message += "<br><br>Version: " + String(VERSION);

     message += F("</form>"
                  "<script>\n");
     message += F("function shwlanpw() {"
      "var x = document.getElementById(\"wlanpw\");"
      "if (x.type === \"password\") {"
      "  x.type = \"text\";"
      "} else {"
      "  x.type = \"password\";"
      "}"
      "}");

    message += F("function shmqttpw() {"
      "var x = document.getElementById(\"mqttpw\");"
      "if (x.type === \"password\") {"
      "  x.type = \"text\";"
      "} else {"
      "  x.type = \"password\";"
      "}"
      "}"      
      "</script>\n");
     message += F("</body></html>");
 webServer.send(200, "text/html", message);
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

void callRoot()
{
  webServer.send(200, "text/html", "<!doctype html><html><head><script>window.onload=function(){window.location.replace('/');}</script></head></html>");
}

//################################################################################################################################################
//##### handleGWupdate
//################################################################################################################################################

void handleGWupdate()
{
  int versuche = 0;
  bool alexamode;
  bool save = false;

  bool alexaaktiv = false;
  String alexatext = "";
  
  for (int z = 0; z < ESPALEXA_MAXDEVICES; z++) 
  {
    if ( webServer.arg("alexaaktiv_" + String(z)) == "1" || z == 0 )
    {
      alexaaktiv = true;
    }
    else
    {
      alexaaktiv = false;
    }
    if ( alexaaktiv != mysetting.alexa[z].aktiv )
    {
      mysetting.alexa[z].aktiv = alexaaktiv;
#ifdef DEBUG
      Serial.printf ("AlexaAktiv %i: %s\n",z,webServer.arg("alexaaktiv_" + String(z)).c_str());
#endif
      save = true;
    }
    
    alexatext = webServer.arg("alexatext_" + String(z));
    if (alexatext.length() > 3 && alexatext != String(mysetting.alexa[z].text))
    {
#ifdef DEBUG
    Serial.printf ("AlexaText %i von %s nach %s geändert\n",z,String(mysetting.alexa[z].text).c_str(), alexatext.c_str());
#endif
    alexatext.toCharArray(mysetting.alexa[z].text,alexatext.length()+1);
    save = true;
    }
  }
    
  if ( save ) saveToEEPROM();
  if ( save ) {
    webServer.send(200, "text/plain", "Reboot wird durchgefuehrt.\nDanach suche mit der Alexa neue Geraete!");
  }
  else
  {
    webServer.send(200, "text/plain", "Reboot wird durchgefuehrt.");
  }
  delay(2000);
  ESP.restart();
}

//################################################################################################################################################
//##### handleWiFiupdate
//################################################################################################################################################

void handleWiFiupdate()
{
  bool save = false;
  int versuche = 0;
  String new_systemname = "";
  String new_wlansidstring = "";
  String new_wlanpwstring = "";
  String new_mqttuser = "";
  String new_mqttpw = "";
  String new_mqttroot = "";
  String new_mqttbroker = "";
  
  new_systemname = webServer.arg("systemname");
  if (new_systemname.length() > 5 && new_systemname != String(mysetting.systemname))
  {
#ifdef DEBUG
    Serial.println ("Sytemname von " + String(mysetting.systemname) + " nach " + webServer.arg("systemname") + " geändert!");
#endif
    new_systemname.toCharArray(mysetting.systemname,new_systemname.length()+1);
    save = true;
  }
  
  new_mqttbroker = webServer.arg("mqttbroker");
  if (new_mqttbroker.length() > 5 && new_mqttbroker != String(mysetting.mqttbroker))
  {
#ifdef DEBUG
    Serial.println ("MQTT-Broker von " + String(mysetting.mqttbroker) + " nach " + webServer.arg("mqttbroker") + " geändert!");
#endif
    new_mqttbroker.toCharArray(mysetting.mqttbroker,new_mqttbroker.length()+1);
    save = true;
  }
  
  new_mqttuser = webServer.arg("mqttuser");
  if (new_mqttuser.length() > 5 && new_mqttuser != String(mysetting.mqttuser))
  {
#ifdef DEBUG
    Serial.println ("MQTT-User von " + String(mysetting.mqttuser) + " nach " + webServer.arg("mqttuser") + " geändert!");
#endif
    new_mqttuser.toCharArray(mysetting.mqttuser,new_mqttuser.length()+1);
    save = true;
  }
  
  new_mqttpw = webServer.arg("mqttpw");
  if (new_mqttpw.length() > 5 && new_mqttpw != String(mysetting.mqttpw))
  {
#ifdef DEBUG
    Serial.println ("MQTT-Passwort von " + String(mysetting.mqttpw) + " nach " + webServer.arg("mqttpw") + " geändert!");
#endif
    new_mqttpw.toCharArray(mysetting.mqttpw,new_mqttpw.length()+1);
    save = true;
  }
  new_mqttroot = webServer.arg("mqttroot");
  if (new_mqttroot.length() > 5 && new_mqttroot != String(mysetting.mqttroot))
  {
#ifdef DEBUG
    Serial.println ("MQTT-Root von " + String(mysetting.mqttroot) + " nach " + webServer.arg("mqttroot") + " geändert!");
#endif
    new_mqttroot.toCharArray(mysetting.mqttroot,new_mqttroot.length()+1);
    save = true;
  }
  
  if ( save ) saveToEEPROM();
  new_wlansidstring = webServer.arg("wlansid");
  if ( new_wlansidstring == "" ) new_wlansidstring = WiFi.SSID();
  
  new_wlanpwstring = webServer.arg("wlanpw");
  
#ifdef DEBUG
  Serial.println ("Aktion: " + webServer.arg("action"));
  Serial.println ("SSID: " + new_wlansidstring);
  Serial.println ("PW: " + new_wlanpwstring);
#endif

  callRoot();
  for (int z = 1; z < 50; z++) 
  { 
    delay(100);
    espalexa.loop();
  }
  delay(1000);
  if (new_wlanpwstring.length() > 6 && new_wlansidstring.length() > 3 )
  {
     WiFi.disconnect(true);
     WiFi.mode(WIFI_STA);
     WiFi.begin(new_wlansidstring, new_wlanpwstring);
     while (WiFi.status() != WL_CONNECTED  && versuche < 20 ) {
     delay(400);
// Flash ESP LED
#ifdef PIN_LED
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
#endif
#ifdef DEBUG_WLAN
    Serial.print(".");
    if ( (versuche+1) %50 == 0 ) Serial.println(".");
#endif
delay(100);
#ifdef PIN_LED
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
#endif
      versuche++;
      WiFi.setAutoReconnect(true);
    }
#ifdef DEBUG
    Serial.println(".");
#endif
    callRoot();
    for (int z = 1; z < 10; z++) 
    { 
      delay(100);
      espalexa.loop();
    }
    
    delay(1000);
  }
  delay(2000);
  ESP.restart();
}


/******************************************************************************
  handle Alexa
******************************************************************************/
void handle_alexa(EspalexaDevice* d)
{
  int alexa_id;
  String mqttroot;
  String mqtttopic;
  String mqttvalue;
  if (d == nullptr) return;
#ifdef DEBUG_ALEXA
    Serial.print("Alexa Setting ändert: Nr: ");
    Serial.print(d->getId() );
    Serial.print(" Prozent: ");
    Serial.println(d->getPercent());
#endif    
    alexa_id = d->getId();
    mysetting.alexa[alexa_idxpoint[alexa_id]].value = d->getPercent();
          
  if ( mqttclient.connected() )
  {
    mqtttopic = String(mysetting.alexa[alexa_idxpoint[alexa_id]].text);
    mqtttopic.replace("/","_");
    mqttroot = String(mysetting.mqttroot) + "/ALEXA/" + mqtttopic; 
    mqttroot.trim();
    mqttroot.toUpperCase();
    mqttroot.replace(" ","_");  
    mqttvalue = String(d->getPercent());
    Serial.printf("Alexa -> MQTT: %s = %s\n",mqttroot.c_str(),mqttvalue.c_str());
    mqttclient.publish(mqttroot.c_str(), mqttvalue.c_str());
  }
  LEDblink();
}


/******************************************************************************
  MQTT
******************************************************************************/
void mqttcallback(char* topic, byte* payload, unsigned int length) {
  String mqtt_value = "";
  String mqtt_rec = String(topic);
  String mqtt_topic;
  String alexa_text;
  mqttloop = 0;
  for (int i = 0; i < length; i++) {
//    Serial.print((char)payload[i]);
    mqtt_value += String((char)payload[i]);
  }
#ifdef DEBUG_MQTT
  Serial.print("MQTT Message arrived ");
  Serial.print(topic);
  Serial.print(" ");
  Serial.println(mqtt_value.c_str());
#endif
  
  mqtt_topic = mqtt_rec.substring(mqtt_rec.lastIndexOf("/ALEXA/")+ 7 );
#ifdef DEBUG_MQTT
  Serial.printf("MQTT TOPIC: %s\n",mqtt_topic.c_str());
#endif  
  alexa_idx = 0;
  for (int z = 0; z <= ESPALEXA_MAXDEVICES; z++) 
  {
    if ( mysetting.alexa[z].aktiv )
    {
      alexa_text = String(mysetting.alexa[z].text);
      alexa_text.trim();
      alexa_text.toUpperCase();
      alexa_text.replace(" ","_");  
      alexa_text.replace("/","_");
      if ( mqtt_topic == alexa_text && mqtt_ready )
      {
        Serial.printf("MQTT -> Alexa: %s = %s\n",alexa_text.c_str(),mqtt_value.c_str());
        mysetting.alexa[z].value = mqtt_value.toInt();
        alexa[alexa_idx]->setPercent(mysetting.alexa[z].value);
        LEDblink();
      }
      alexa_idx++;
    }
  }
  LEDblink();
}

void mqttreconnect() {
  // Loop until we're reconnected
  String mqttroot;
  String strIP = String(myIP[0]) + '.' + String(myIP[1]) + '.' + String(myIP[2]) + '.' + String(myIP[3]);
  String sname =  mysetting.systemname;

  if (!mqttclient.connected() && wifimode )
  {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (mqttclient.connect(mysetting.systemname,mysetting.mqttuser, mysetting.mqttpw)) {

      Serial.println("connected");

      mqttroot = String(mysetting.mqttroot) + "/IP";
      mqttroot.trim();
      mqttroot.toUpperCase();
      mqttroot.replace(" ","_");   
#ifdef DEBUG_MQTT
      Serial.printf("publish MQTT-ROOT: %s = %s\n",mqttroot.c_str(), strIP.c_str() );
#endif  
      mqttclient.publish(mqttroot.c_str(), strIP.c_str());
      mqttroot = String(mysetting.mqttroot) + "/SYSTEMNAME";
      mqttroot.trim();
      mqttroot.toUpperCase();
      mqttroot.replace(" ","_");   
#ifdef DEBUG_MQTT
      Serial.printf("publish MQTT-ROOT: %s = %s\n",mqttroot.c_str(), sname.c_str());
#endif  
      mqttclient.publish(mqttroot.c_str(), sname.c_str());
      // ... and resubscribe
    
      mqttroot = String(mysetting.mqttroot) + "/ALEXA/#";
      mqttroot.trim();
      mqttroot.toUpperCase();
      mqttroot.replace(" ","_");   
#ifdef DEBUG_MQTT
      Serial.printf("subscribe MQTT-ROOT: %s\n",mqttroot.c_str());
#endif  
      mqttclient.subscribe(mqttroot.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.println(mqttclient.state());
    }
  }
}

void LEDblink() {
  for (int z = 0; z <= 5; z++) 
  {
#ifdef PIN_LED
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
#endif
    delay (40);
  }
}

/******************************************************************************
  Webserver
******************************************************************************/

void setupWebServer()
{
  webServer.on("/", handleRoot);
  webServer.on("/gwupdate", handleGWupdate);
  webServer.on("/handlewifisettings", handleWiFiSettings);
  webServer.on("/wifiupdate", HTTP_POST, handleWiFiupdate);
  
  webServer.onNotFound([]() {
      if (!espalexa.handleAlexaApiCall(webServer.uri(),webServer.arg(0))) //if you don't know the URI, ask espalexa whether it is an Alexa control request
      {
        if (!handleCaptivePortal(webServer.uri()))                     // send it if it exists
        {
          //whatever you want to do with 404s
          webServer.send(404, "text/plain", "Not found");
        }
      }
  });
#ifdef DEBUG
  Serial.println("HTTP server started");
#endif
}

//################################################################################################################################################
//##### handleCaptivePortal
//################################################################################################################################################

bool handleCaptivePortal(String path)
{
  if (path.startsWith("/generate_204") || path.startsWith("/fwlink") ) 
  {
    if (webServer.hostHeader() != String(mysetting.systemname) ) {
#ifdef DEBUG
      Serial.print ("captive portal: ");
      Serial.println ( webServer.hostHeader());
#endif
      webServer.sendHeader("Location", String("http://" + String(mysetting.systemname) + "/handlewifisettings" ), true);
      webServer.send(302, "text/plain", "");
      webServer.client().stop();
    }
    else
    {
      handleRoot;
    }
    return true;
  }
#ifdef DEBUG
  Serial.println("\tNot Found: " + path);
#endif
  return false;
}
