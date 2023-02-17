#pragma once
#include <Arduino.h>
#include "SPIFFS.h"

bool readConfigFile(const char path[], void callback(String key, String value) );
