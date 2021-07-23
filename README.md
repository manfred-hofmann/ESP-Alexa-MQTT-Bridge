# ESP-Alexa-MQTT-Bridge  
Mit Alexa Befehl MQTT publishen  
Stellt für die Alexa bis zu 30 Geräte zur Verfügung.  
(Geräte sind vom Typ Philips Hue)  
Die Sprachbefehle werden mittels MQTT an einen MQTT-Server  
(z.B. ioBroker mit MQTT-Adapter) gepublished.  


## Inbetriebnahme: 
  
Den Code so wie er ist per Arduino-IDE auf einen ESP8266 übertragen.  
Wie das geht kann man im in zahlreichen Beiträgen im Internet erfahren.  
   
Folgende Libraries werden zusätzlich benötigt:
  
Espalexa:  
Open your Arduino IDE and go to Sketch > Include Library > Manage Libraries. The Library Manager should open.  
Search for "Espalexa" on the Search box and install the Espalexa library by Christian Schwinne. --> Espalexa.h (Version 2.7.0)  
  
PubSubClient:  
Open your Arduino IDE and go to Sketch > Include Library > Manage Libraries. The Library Manager should open.  
Search for "PubSubClient" on the Search box and install the MQTT library by Nick O'Leary. --> PubSubClient.h (Version 2.8.0)  
   
Nach dem Start des ESP wird ein offener WiFi AP erstellt. Dauert bis zu 3 Minuten!    
Suche mit dem Handy das WLAN ESP_ALEXA_GW und melde Dich im WLAN an.  
Dann öffnet sich der Browser in dem die WLAN Parameter des lokalen Netzwerkes und die MQTT Parameter angegeben werden können.  
folgende Einstellungen sind hier zu setzen:  
  
*Systemname: Netzwerkname des ESP  
*SID: WLAN Parameter des lokalen Netzwerks  
*MQTT Server: Name oder IP-Adresse des MQTT Servers (z.B. ioBroker mit MQTT-Adapter)  
*MQTT User/Passwort: User und Passort des MQTT Server Zugang.  
*MQTT ROOT: Root Hierarchieknoten im MQTT Server wo die Nachrichten abgelegt werden.  
  
![WLan/MQTT-Einstellungen](https://github.com/manfred-hofmann/ESP-Alexa-MQTT-Bridge/blob/main/pic/web03.JPG "WLan/MQTT-Einstellungen")  
  
  
Nachdem Speichern wird der ESP neu gestartet!  
Nun sollte sich der ESP mit dem lokalen Netzwerk verbinden und  
im MQTT-Server werden die Knoten IP und SYSTEMNAME unter dem Root Hierarchieknoten angelegt.
Außerdem wird der erste Eintrag in der Befehlsliste angelegt z.B. TEST:
![MQTT-Server](https://github.com/manfred-hofmann/ESP-Alexa-MQTT-Bridge/blob/main/pic/mqtt01.JPG "MQTT-Server")    
  
Als nächstes muss die Alexa nach neuen Geräten suchen.  
(entweder per APP oder per Sprachbefehl "Alexe suche Geräte")  
   
In der APP sollte ein neues Gerät gefunden werden (z.B. Test)  
<img src="https://github.com/manfred-hofmann/ESP-Alexa-MQTT-Bridge/blob/main/pic/alexa01.jpg" width="200" height="420" />   
Wenn das Gerät (Test) nun durch einen Sprachbefehl geändert wird, wird das an den MQTT-Server weitergegeben.  
  
Auf der Hauptseite des ESP können nun weitere Geräte eingerichtet werden:  
![Web01](https://github.com/manfred-hofmann/ESP-Alexa-MQTT-Bridge/blob/main/pic/web01.JPG "Web01")  
![Web02](https://github.com/manfred-hofmann/ESP-Alexa-MQTT-Bridge/blob/main/pic/web02.JPG "Web02")  
  
Nachdem Speichern wird jedesmal ein Neustart des ESP durchgeführt!  
Der Neustart dauert aber nur wenige Sekunden.  
   
Wenn neue Geräte hinzugefügt wurden, müssen diese danach immer von der Alexa gesucht werden.  
  
  
Viel Spaß damit!  

 
