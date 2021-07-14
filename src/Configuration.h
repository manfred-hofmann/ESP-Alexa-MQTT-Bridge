//******************************************************************************
// Configuration.h
//******************************************************************************
#ifndef CONFIGURATION_H
#define CONFIGURATION_H


//******************************************************************************
// folgende Werte bitte anpassen:
#define PIN_LED        LED_BUILTIN
#define ESPALEXA_MAXDEVICES     30      // max. Anzahl von Alexa Geräten
#define OTA_PASS "1234"                 // OTA Passwort (für die Übertragung des SPIFFS auskommentieren!)
#define DEFAULT_HOSTNAME "ESP_ALEXA_GW" // Default Hostname kann in WiFi Settings neu gesetzt werden
#define DEFAULT_MQTTBROKER "iobroker"   // Default Name oder IP des IO-Brokers kann in WiFi Settings neu gesetzt werden
#define DEFAULT_MQTTUSER "mqttuser"     // Default MQTT User kann in WiFi Settings neu gesetzt werden
#define DEFAULT_MQTTPASSWORD "mqttpass" // Default MQTT Passwot kann in WiFi Settings neu gesetzt werden
#define DEFAULT_MQTTROOT " ESP_GW01"    // Default MQTT Root kann in WiFi Settings neu gesetzt werden

//******************************************************************************
//DEBUG:
//#define DEBUG
#define DEBUG_WLAN
//#define DEBUG_MQTT
//#define DEBUG_ALEXA

//******************************************************************************
#endif
