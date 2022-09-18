#include <Arduino.h>
#include <string.h>
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

extern TaskHandle_t hTaskHW;
extern TaskHandle_t hTaskWeb;

#include "HAL_LX790.h"

void TaskHW( void * pvParameters )
{
  unsigned long time;

  // init WiFi
  bool  WiFiConnected = false;
  unsigned long lastWifiUpdate = 0;
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname);
  WiFi.begin(ssid, password);

  // init HW Communication
  LX790_State state;
  unsigned long lastStateUpdate = 0;
  memset(&state, 0x00, sizeof state);
  state.digits[0]='#'; state.digits[1]='#'; state.digits[2]='#'; state.digits[3]='#'; state.point=' ';
  state.msg = "";  
  HAL_setup();
  state.updated = true;

  // command handling
  CMD_Type cmd = {CMD_Type::NA, 0};
  unsigned long cmdStart = 0;

  while(1)
  {
    time = millis();

    // check WLAN state
    wl_status_t wifiStatus = WiFi.status();
    state.wifi = (wifiStatus == WL_CONNECTED);
    if ( WiFiConnected )
    {
      if ( (time - lastWifiUpdate) > 5000)
      {
        lastWifiUpdate = time;

        if (wifiStatus != WL_CONNECTED)
        {
          WiFiConnected = false;
          Serial.println(F("WiFi reconnect.."));
          WiFi.disconnect();
          WiFi.begin(ssid, password);
        }
      }
    }
    else
    {
      if (wifiStatus == WL_CONNECTED)
      {
        WiFiConnected = true;
        Serial.print  (F("WiFi successfully connected with IP: "));
        Serial.println(WiFi.localIP());
      }
      else if ( (time - lastWifiUpdate) > 1000)
      {
        lastWifiUpdate = time;
        Serial.println(F("WiFi connect.."));
      }
    }

    // do HW communication
    HAL_loop(state);

    // handle command queue
    if ( !cmd.cmd ) {
      // get next command if we have no active command handled
      if ( xQueueReceive(cmdQueue, &cmd, 0) == pdPASS ) {
        cmdStart = time;
        state.updated = true;
      } else {
        cmd.cmd = CMD_Type::NA;
      }
    }
    switch ( cmd.cmd ) {
      case CMD_Type::NA:
        // nothing to do
        break;

      case CMD_Type::WAIT:
        if ( (time - cmdStart) > cmd.param ) {
          // finished
          cmd.cmd = CMD_Type::NA;
        }
        break;

      case CMD_Type::REBOOT:
        if ( (time - cmdStart) > cmd.param ) {
          // finished
          cmd.cmd = CMD_Type::NA;
          ESP.restart();
        }
        break;

      case CMD_Type::BTN_PRESS:
        HAL_buttonPress(static_cast<BUTTONS>(cmd.param));
        cmd.cmd = CMD_Type::NA;
        break;

      case CMD_Type::BTN_RELEASE:
        HAL_buttonRelease(static_cast<BUTTONS>(cmd.param));
        cmd.cmd = CMD_Type::NA;
        break;
    }

    // sync state
    if ( state.updated || ( (time - lastStateUpdate) > 2000) ) {
      lastStateUpdate = time;
      decodeDisplay(state);

      xQueueSend(stateQueue, &state, 0);
      state.updated = false;
    }

    delay(5);
  }
}


