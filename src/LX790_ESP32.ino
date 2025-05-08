#include <Arduino.h>
#include <string.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "LX790_util.h"

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
  Serial.println("\n\nLX79x_WiFi application started");
  Serial.print("Build date: "); Serial.print  (__DATE__);
  Serial.print(" time: "); Serial.println(__TIME__);

  if(!SPIFFS.begin(true))
    Serial.println(F("init SPIFFS error"));
  
  // read config
  config.captivePortal = false;
  config.portalTimeout = 10 * 60;

  if (!SPIFFS.exists("/index.html")) {
    Serial.println(F("Failed to read filesystem"));
    Serial.println(F("Build & upload Filesystem Image first!"));
    while(1) {
      delay(1000);
    }	
  }

  File file = SPIFFS.open("/config.json", "r");

  // deserialize the JSON document
  JsonDocument doc;
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

  DEBUG_println("config");
  DEBUG_print("  name: "); DEBUG_println(config.hostname);
  DEBUG_print("  pin: "); DEBUG_println(config.pin);
  DEBUG_print("  wifiEnabled: "); DEBUG_println(config.wifiEnabled);
  DEBUG_print("  wifiSSID: "); DEBUG_println(config.wifiSSID);
  DEBUG_print("  wifiPassword: "); DEBUG_println(config.wifiPassword);
  DEBUG_print("  captivePortal: "); DEBUG_println(config.captivePortal);
  DEBUG_print("  portalTimeout: "); DEBUG_println(config.portalTimeout);
  DEBUG_print("  portalPassword: "); DEBUG_println(config.portalPassword);
  DEBUG_println("");

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

  DEBUG_println("start TaskHW");
  xTaskCreatePinnedToCore(
    TaskHW,   // Function to implement the task -> I2C, WiFi
    "TaskHW", // Name of the task
    10000,   // Stack size in words
    NULL,    // Task input parameter
    1,       // Priority of the task
    &hTaskHW, // Task handle
    0);      // Core where the task should run

  delay(1000);

  DEBUG_println("start TaskWeb");
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
