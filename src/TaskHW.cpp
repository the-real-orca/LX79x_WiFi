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
  memset(&state, 0x00, sizeof state);
  state.digits[0]='#'; state.digits[1]='#'; state.digits[2]='#'; state.digits[3]='#'; state.point=' ';
  state.msg = "";  
  HAL_setup();
  state.updated = true;

  while(1)
  {
    // check WLAN state
    wl_status_t wifiStatus = WiFi.status();
    state.wifi = (wifiStatus == WL_CONNECTED);
    if (WiFi_WasConnected)
    {
      if (millis() - Lst_WiFi_Status > 10000)
      {
        Lst_WiFi_Status = millis();

        if (wifiStatus != WL_CONNECTED)
        {
          Serial.println(F("WLAN reconnect.."));
          WiFi.disconnect();
          WiFi.begin(ssid, password);
        }
      }
    }
    else
    {
      if (wifiStatus == WL_CONNECTED)
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

    // sync state
    if ( state.updated ) {
      decodeDisplay(state);
  
/*  
      std::lock_guard<std::mutex> lock(stateMutex);
      memcpy(&stateShared, &state, sizeof state);
      state.updated = false;
*/      
    }


    // @TODO handle buttons 


    delay(1);
  }
}


