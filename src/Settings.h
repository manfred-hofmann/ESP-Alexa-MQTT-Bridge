//******************************************************************************
// Settings.h
//******************************************************************************

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <EEPROM.h>
#include "Configuration.h"

#define SETTINGS_VERSION 1
typedef struct
  {
    bool aktiv;
    char text[30];
    int  value;
  } s_alexaparameter;

typedef struct
  {
    uint8_t version;
    char systemname[30];
    uint8_t alexa_anzahl;
    char mqttbroker[50];
    char mqttuser[30];
    char mqttpw[30];
    char mqttroot[30];
    s_alexaparameter alexa[ESPALEXA_MAXDEVICES+1];
  } s_mysettings;


void resetToDefault();
void saveToEEPROM();
void loadFromEEPROM();

#endif
