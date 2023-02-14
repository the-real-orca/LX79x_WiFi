#include <Arduino.h>
#include <string.h>
#include "LX790_util.h"
#include "SPIFFS.h"

// Copy "config_sample.h" to "config.h" and change it according to your setup.
#include "config.h"


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
    
File file = SPIFFS.open("/config.ini", "r");
if (!file) {
  Serial.println("cannot open 'config.ini'");
  return;
} else {
  Serial.println("config file:");
  while (file.available()) {
    String line = file.readStringUntil('\n');
    int pos = line.indexOf('=');
    if ( pos > 0 ) {
      String key = line.substring(0, pos); key.trim();
      String value = line.substring(pos+1); value.trim();
      if ( key.equalsIgnoreCase("SSID") ) {
        config.ssid = value;
      } else if ( key.equalsIgnoreCase("PASSWORD") ) {
        config.wifi_pwd = value;
      } else if ( key.equalsIgnoreCase("HOSTNAME") ) {
        config.hostname = value;
      } else if ( key.equalsIgnoreCase("PIN") ) {
        strncpy(config.pin, value.c_str(), 4);
      }

    }
  }
  file.close();
}


Serial.print("SSID: "); Serial.println(config.ssid);
Serial.print("wifi_pwd: "); Serial.println(config.wifi_pwd);
Serial.print("hostname: "); Serial.println(config.hostname);
Serial.print("pin: "); Serial.println(config.pin);


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
