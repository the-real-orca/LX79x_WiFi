#include <Arduino.h>
#include <string.h>
#include "LX790_util.h"

// Copy "config_sample.h" to "config.h" and change it according to your setup.
#include "config.h"
#include "config_util.h"


TaskHandle_t hTaskHW;   //Hardware: I2C, WiFi...
TaskHandle_t hTaskWeb;   //Web...

void TaskHW( void * pvParameters );
void TaskWeb( void * pvParameters );

SemaphoreHandle_t stateQueue = NULL;
QueueHandle_t cmdQueue = NULL;

LX790_Config config;

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  Serial.print("Build date: "); Serial.print  (__DATE__);
  Serial.print(" time: "); Serial.println(__TIME__);

  if(!SPIFFS.begin(true))
    Serial.println(F("init SPIFFS error"));
  
  // read config
  config.captivePortal = false;
  config.portalTimeout = 10 * 60;

  if ( !readConfigFile("/config.ini", [](String key, String value) {
    if ( key.equalsIgnoreCase("SSID") ) {
      config.ssid = value;
    } else if ( key.equalsIgnoreCase("PASSWORD") ) {
      config.wifi_pwd = value;
    } else if ( key.equalsIgnoreCase("HOSTNAME") ) {
      config.hostname = value;
    } else if ( key.equalsIgnoreCase("PIN") ) {
      strncpy(config.pin, value.c_str(), 4);
    } else if ( key.equalsIgnoreCase("CAPTIVEPORTAL") ) {
      config.captivePortal = value.equalsIgnoreCase("TRUE") || value.equalsIgnoreCase("1");
    } else if ( key.equalsIgnoreCase("CAPTIVE") ) {
      config.captivePortal = value.equalsIgnoreCase("TRUE") || value.equalsIgnoreCase("1");
    } else if ( key.equalsIgnoreCase("PORTAL") ) {
      config.captivePortal = value.equalsIgnoreCase("TRUE") || value.equalsIgnoreCase("1");
    } else if ( key.equalsIgnoreCase("PORTALTIMEOUT") ) {
      config.portalTimeout = value.toInt();
    }
  }) ) {
    config.captivePortal = true;
  }

//FIXME
config.captivePortal = true;


#if DEBUG_SERIAL_PRINT
  Serial.print("SSID: "); Serial.println(config.ssid);
  Serial.print("wifi_pwd: "); Serial.println(config.wifi_pwd);
  Serial.print("hostname: "); Serial.println(config.hostname);
  Serial.print("pin: "); Serial.println(config.pin);
  Serial.print("captivePortal: "); Serial.println(config.captivePortal);
  Serial.print("portalPassword: "); Serial.println(config.portalPassword);
  Serial.print("portalTimeout: "); Serial.println(config.portalTimeout);
#endif

  stateQueue = xQueueCreate(2, sizeof(LX790_State));
  if (stateQueue == NULL) {
    Serial.println(F("init state queue error"));
    while(1) {}
  }

  cmdQueue = xQueueCreate(16, sizeof(CMD_Type));
  if (cmdQueue == NULL) {
    Serial.println(F("init command queue error"));
    while(1) {}
  }

  xTaskCreatePinnedToCore(
    TaskHW,   // Function to implement the task -> I2C, WiFi
    "TaskHW", // Name of the task
    10000,   // Stack size in words
    NULL,    // Task input parameter
    1,       // Priority of the task
    &hTaskHW, // Task handle
    0);      // Core where the task should run

  delay(500);

  xTaskCreatePinnedToCore(
    TaskWeb,   // Function to implement the task -> Webserver
    "TaskWeb", // Name of the task
    10000,   // Stack size in words
    NULL,    // Task input parameter
    1,       // Priority of the task
    &hTaskWeb, // Task handle
    1);      // Core where the task should run

}

void loop()
{
  //Core 1
  delay(1000);  // prevent loop from eating up resources
}
