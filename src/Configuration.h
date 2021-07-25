//******************************************************************************
// Configuration.h
//******************************************************************************
#ifndef CONFIGURATION_H
#define CONFIGURATION_H


//******************************************************************************
// folgende Werte bitte anpassen:
#define WIFI_SETUP_TIMEOUT                60  // WIFI Timeout. Solange wird versucht eine Verbindung mit dem WLAN herzustellen
#define AP_WIFI_TIMEOUT                  300  // Solange wartet der WIFI AP auf Eingaben
#define PIN_LED                  LED_BUILTIN  // OnBoard LED
#define ESPALEXA_MAXDEVICES               30  // max. Anzahl von Alexa Geräten
#define OTA_PASS                      "1234"  // OTA Passwort
#define DEFAULT_HOSTNAME      "ESP_ALEXA_GW"  // Default Hostname kann in WiFi Settings neu gesetzt werden
#define DEFAULT_MQTTBROKER        "iobroker"  // Default Name oder IP des IO-Brokers kann in WiFi Settings neu gesetzt werden
#define DEFAULT_MQTTUSER          "mqttuser"  // Default MQTT User kann in WiFi Settings neu gesetzt werden
#define DEFAULT_MQTTPASSWORD      "mqttpass"  // Default MQTT Passwort kann in WiFi Settings neu gesetzt werden
#define DEFAULT_MQTTROOT          "ESP_GW01"  // Default MQTT Root kann in WiFi Settings neu gesetzt werden

//******************************************************************************
//DEBUG:
//#define DEBUG
#define DEBUG_WLAN
//#define DEBUG_MQTT
#define DEBUG_ALEXA

//******************************************************************************
#endif
