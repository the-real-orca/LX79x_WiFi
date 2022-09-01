#include <Arduino.h>
#include <string.h>

// Copy "config_sample.h" to "config.h" and change it according to your setup.
#include "config.h"


TaskHandle_t hTaskHW;   //Hardware: I2C, WiFi...
TaskHandle_t hTaskWeb;   //Web...

void TaskHW( void * pvParameters );
void TaskWeb( void * pvParameters );

const char* Buttons[] = {"I/O", "start", "home", "ok", "stop", nullptr};

SemaphoreHandle_t SemMutex;

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  Serial.print  (F("Build date: "));
  Serial.print  (__DATE__);
  Serial.print  (F(" time: "));
  Serial.println(__TIME__);

  SemMutex = xSemaphoreCreateMutex();
  if (SemMutex == NULL)
    Serial.println(F("init semaphore error"));

  xTaskCreatePinnedToCore(
    TaskHW,   /* Function to implement the task -> I2C, WiFi*/
    "TaskHW", /* Name of the task */
    10000,   /* Stack size in words */
    NULL,    /* Task input parameter */
    1,       /* Priority of the task */
    &hTaskHW, /* Task handle. */
    0);      /* Core where the task should run */

  delay(500);

  xTaskCreatePinnedToCore(
    TaskWeb,   /* Function to implement the task -> Webserver*/
    "TaskWeb", /* Name of the task */
    10000,   /* Stack size in words */
    NULL,    /* Task input parameter */
    1,       /* Priority of the task */
    &hTaskWeb, /* Task handle. */
    1);      /* Core where the task should run */
}

void loop()
{
  //Core 1
  delay(1000);  // prevent loop from eating up resources
}
