#include <Arduino.h>
#include <string.h>
#include <Wire.h>
//#include "CRC16.h"
#include "CRC.h"
#include "ahWireSlave.h"
#include <WiFi.h>
#include "LX790_util.h"

#include "config.h"
#ifndef SSID
  #error "Copy 'config_sample.h' to 'config.h' and change it according to your setup."
#endif

//WiFI is set in "config.h"
const char* ssid     = SSID;
const char* password = PASSWORD;
const char* hostname = HOSTNAME;

#include "HAL_LX790.h"

void TaskHW( void * pvParameters )
{
  // init WiFi
  int WiFi_WasConnected = 0;
  unsigned long Lst_WiFi_Status = 0;
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname);
  WiFi.begin(ssid, password);
  Lst_WiFi_Status = millis();

  // init HW Communication
  LX790_State state;
  HAL_setup();
 
  while(1)
  {
    // check WLAN state
    if (WiFi_WasConnected)
    {
      if (millis() - Lst_WiFi_Status > 10000)
      {
        Lst_WiFi_Status = millis();

        if (WiFi.status() != WL_CONNECTED)
        {
          Serial.println(F("WLAN reconnect.."));
          WiFi.disconnect();
          WiFi.begin(ssid, password);
        }
      }
    }
    else
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        WiFi_WasConnected = 1;
        Serial.print  (F("WiFi successfully connected with IP: "));
        Serial.println(WiFi.localIP());
      }
      else if (millis() - Lst_WiFi_Status > 1000)
      {
        Lst_WiFi_Status = millis();
        Serial.println(F("WiFi connect.."));
      }
    }

    // do HW communication
    HAL_loop(state);
    decodeDisplay(state);

    // handle buttons @TODO

    // sync state @TODO

    delay(1);
  }
}


