//******************************************************************************
// Settings.cpp
//******************************************************************************

#include "Settings.h"
extern s_mysettings mysetting;

void resetToDefault()
{
  String alexa_wort;
#ifdef DEBUG
  Serial.println("Reset to Default...");
#endif
  mysetting.version = SETTINGS_VERSION;
  strcpy (mysetting.systemname,DEFAULT_HOSTNAME);
  mysetting.alexa_anzahl = 1;
  strcpy (mysetting.mqttbroker,DEFAULT_MQTTBROKER);
  strcpy (mysetting.mqttuser,DEFAULT_MQTTUSER);
  strcpy (mysetting.mqttpw,DEFAULT_MQTTPASSWORD);
  strcpy (mysetting.mqttroot,DEFAULT_MQTTROOT);
  mysetting.alexa[0].aktiv = true;
  strcpy (mysetting.alexa[0].text,"Test");
  mysetting.alexa[0].value = 0;
 

  for ( int a=1; a <= ESPALEXA_MAXDEVICES; a++ )
  {
     mysetting.alexa[a].aktiv = false;
     strcpy (mysetting.alexa[a].text,("Alexa-Wort " + String(a)).c_str());
     mysetting.alexa[a].value = 0;
  }
}

void loadFromEEPROM()
{
#ifdef DEBUG
    Serial.println("Load from EEPROM.");
#endif
    EEPROM.begin(3000);
    EEPROM.get(0, mysetting);
    if (mysetting.version != SETTINGS_VERSION)
    {
      resetToDefault();
      saveToEEPROM();
    }
#ifdef DEBUG
    Serial.printf("EEPROM version: %i\n", mysetting.version);
#endif
    EEPROM.end();
}

void saveToEEPROM()
{
#ifdef DEBUG
  Serial.println("Settings saved to EEPROM.");
#endif
  EEPROM.begin(3000);
  EEPROM.put(0, mysetting);
    //EEPROM.commit();
  EEPROM.end();
}
