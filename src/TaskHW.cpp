#include <Arduino.h>
#include <string.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include <DNSServer.h>
#include <mDNS.h>

#include "LX790_util.h"
#include "EEPROM.h"

#include "config.h"

extern TaskHandle_t hTaskHW;
extern TaskHandle_t hTaskWeb;
DNSServer dnsServer;

#include "HAL_LX790.h"


void TaskHW( void * pvParameters )
{
  unsigned long time;
  String commandBuffer;

  // init WiFi
  bool WiFiConnected = false;
  unsigned long lastWifiUpdate = -10000;
  WiFi.mode(WIFI_OFF);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE); 
  WiFi.setHostname(config.hostname.c_str());

  // init HW Communication
  LX790_State state;
  unsigned long lastStateUpdate = 0;
  memset(&state, 0x00, sizeof state);
  state.digits[0]='#'; state.digits[1]='#'; state.digits[2]='#'; state.digits[3]='#'; state.point=' ';
  state.msg = "";
  state.hostname = config.hostname.c_str();

  EEPROM.begin(EEPROM_SIZE);
  state.autoUnlock = EEPROM.readBool(EEPROM_autoUnlock);
  state.debugLog = EEPROM.readBool(EEPROM_debugLog);


  HAL_setup();
  state.updated = true;

  // command handling
  CMD_Type cmd = {CMD_Type::NA, 0};
  unsigned long cmdStart = 0;

  while(1)
  {
    time = millis();

    if (config.wifiEnabled) {
      // check WLAN state
      wl_status_t wifiStatus = WiFi.status();
      state.wifi = (wifiStatus == WL_CONNECTED);

      // WiFi Network management
      if (config.captivePortal)
      {
        // captive portal
        if ( WiFiConnected )
        {
          // handle DNS
          dnsServer.processNextRequest();

          // check if captive portal timed out
          wifi_sta_list_t wifi_sta_list;
          esp_wifi_ap_get_sta_list(&wifi_sta_list);
          if ( ((time - lastWifiUpdate) > config.portalTimeout*1000L ) && !wifi_sta_list.num ) // close captive portal on timeout and no active client connection
          { 
            Serial.println("captive portal timed out");
            dnsServer.stop();
            WiFi.softAPdisconnect();
            config.captivePortal = false;
            WiFiConnected = false;
            lastWifiUpdate = -10000;
          }
        }
        else
        {   
          // start AP
          Serial.println(F("start AP.."));
          WiFi.mode(WIFI_AP);
          delay(100);
          if ( !WiFi.softAP(config.hostname.c_str(), config.portalPassword.c_str()) ) {
            Serial.println("softAP failed");
          }
          delay(100);

          Serial.print("AP SSID: "); Serial.println(config.hostname);
          Serial.print("AP password: "); Serial.println(config.portalPassword);
          Serial.print("AP IP address: "); Serial.println(WiFi.softAPIP());

          // Setup the DNS server redirecting all the domains
          dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
          dnsServer.start(53, "*", WiFi.softAPIP());

          WiFiConnected = true;
          lastWifiUpdate = time;
        }

      } else {
        // connect as WiFi client
        if ( WiFiConnected )
        {
          if ( (time - lastWifiUpdate) > 5000)
          {
            lastWifiUpdate = time;

            if (wifiStatus != WL_CONNECTED)
            {
              WiFiConnected = false;
              Serial.println("WiFi reconnect..");
              WiFi.disconnect();
            }
          }
        }
        else
        {
          if (wifiStatus == WL_CONNECTED)
          {
            WiFiConnected = true;
            Serial.print("WiFi successfully connected with IP: "); Serial.println(WiFi.localIP());
          }
          else if ( (time - lastWifiUpdate) > 4000)
          {
            lastWifiUpdate = time;
            Serial.println("WiFi connect..");       
            WiFi.mode(WIFI_STA);
            WiFi.begin(config.wifiSSID.c_str(), config.wifiPassword.c_str());
          }
        }
      }
    } else {
      state.wifi = false;
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
          // finished wait
          cmd.cmd = CMD_Type::NA;
        }
        break;

      case CMD_Type::REBOOT:
        if ( (time - cmdStart) > cmd.param ) {
          // finished timeout
          DEBUG_println("reboot ESP32");
          cmd.cmd = CMD_Type::NA;
          ESP.restart();
        }
        break;

      case CMD_Type::DISCONNECT:
        // disconnet network
        DEBUG_println("disconnet network");
        if ( WiFi.getMode() == WIFI_AP ) {
          dnsServer.stop();
          WiFi.softAPdisconnect();

        } else {
          WiFi.disconnect();
        }
        config.wifiEnabled = false;
        WiFiConnected = false;
        lastWifiUpdate = -10000;
        cmd.cmd = CMD_Type::NA;
        break;

      case CMD_Type::WIFI_CLIENT:
        // enable client mode
        DEBUG_println("enable client mode");
        config.wifiEnabled = true;
        config.captivePortal = false;
        WiFiConnected = false;
        lastWifiUpdate = -10000;
        cmd.cmd = CMD_Type::NA;
        break;

      case CMD_Type::WIFI_PORTAL:
        // enable captive portal
        DEBUG_println("enable captive portal");
        config.wifiEnabled = true;
        config.captivePortal = true;
        WiFiConnected = false;
        lastWifiUpdate = -10000;
        cmd.cmd = CMD_Type::NA;
        break;

      case CMD_Type::BTN_PRESS:
        HAL_buttonPress(static_cast<BUTTONS>(cmd.param));
        cmd.cmd = CMD_Type::NA;
        break;

      case CMD_Type::BTN_RELEASE:
        HAL_buttonRelease(static_cast<BUTTONS>(cmd.param));
        cmd.cmd = CMD_Type::NA;
        break;

      case CMD_Type::DEBUGLOG:
        if ( state.debugLog != cmd.param ) {
          state.debugLog = cmd.param;
          EEPROM.writeBool(EEPROM_debugLog, state.debugLog);
          EEPROM.commit();
        }
        cmd.cmd = CMD_Type::NA;
        break;

      case CMD_Type::AUTOUNLOCK:
        if ( state.autoUnlock != cmd.param ) {
          state.autoUnlock = cmd.param;
          EEPROM.writeBool(EEPROM_autoUnlock, state.autoUnlock);
          EEPROM.commit();
        }
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

    // process Serial input
    if ( Serial.available() ) {
      char inputChar = Serial.read();

      if (inputChar == '\n' || commandBuffer.length() > 32 ) {
        commandBuffer.trim();

        // Execute the command
        if ( commandBuffer.equalsIgnoreCase("reboot") ) {
          // reboot ESP32
          CMD_Type xcmd({CMD_Type::REBOOT, 0}); xQueueSend(cmdQueue, &xcmd, 0);
        } else if ( commandBuffer.equalsIgnoreCase("portal") || commandBuffer.equalsIgnoreCase("ap") ) {
          CMD_Type xcmd({CMD_Type::DISCONNECT, 0}); xQueueSend(cmdQueue, &xcmd, 0);
          xcmd = {CMD_Type::WAIT, 1000}; xQueueSend(cmdQueue, &xcmd, 0);
          xcmd = {CMD_Type::WIFI_PORTAL, 0}; xQueueSend(cmdQueue, &xcmd, 0);
        } else if ( commandBuffer.equalsIgnoreCase("client") ) {
          CMD_Type xcmd({CMD_Type::DISCONNECT, 0}); xQueueSend(cmdQueue, &xcmd, 0);
          xcmd = {CMD_Type::WAIT, 1000}; xQueueSend(cmdQueue, &xcmd, 0);
          xcmd = {CMD_Type::WIFI_CLIENT, 0}; xQueueSend(cmdQueue, &xcmd, 0);
        } else if ( commandBuffer.equalsIgnoreCase("disconnect") ) {
          CMD_Type xcmd({CMD_Type::DISCONNECT, 0}); xQueueSend(cmdQueue, &xcmd, 0);
        } else if ( commandBuffer.equalsIgnoreCase("config") ) {
          Serial.print("wifiSSID: "); Serial.println(config.wifiSSID);
          Serial.print("wifiPassword: "); Serial.println(config.wifiPassword);
          Serial.print("hostname: "); Serial.println(config.hostname);
          Serial.print("pin: "); Serial.println(config.pin);
          Serial.print("captivePortal: "); Serial.println(config.captivePortal);
          Serial.print("portalPassword: "); Serial.println(config.portalPassword);
          Serial.print("portalTimeout: "); Serial.println(config.portalTimeout);
        } else {
          // print help
          Serial.println("Serial commands:");
          Serial.println(" reboot:      reboots ESP32");
          Serial.println(" config:      show config settings");
          Serial.println(" portal | ap: start captive portal");
          Serial.println(" client:      switch to client mode");
          Serial.println(" disconnect:  deactivate WiFi");
          Serial.println();
        }

        commandBuffer.clear();
      } else {
        commandBuffer += inputChar;
        Serial.print(inputChar);
      }
    }

    delay(5);
  }
}


