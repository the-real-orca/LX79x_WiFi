#include <Arduino.h>
#include <string.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "LX790_util.h"

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
  
  // read config
  config.captivePortal = false;
  config.portalTimeout = 10 * 60;

  File file = SPIFFS.open("/config.json", "r");

  // deserialize the JSON document
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // import config
  config.hostname = doc["name"] | "LX790";
  strncpy(config.pin, doc["pin"] | "1234", 4);
  config.wifiEnabled = doc["wifiEnabled"] | true;
  config.wifiSSID = doc["wifiSSID"] | "SSID";
  config.wifiPassword = doc["wifiPassword"] | "";
  config.captivePortal = doc["captivePortal"] | true;
  config.portalTimeout = doc["portalTimeout"] | 600;
  config.portalPassword = doc["portalPassword"] | "";
  file.close();

// TODO enable captive portal on "HOME" button press
// TODO upload config.json via serial terminal

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
