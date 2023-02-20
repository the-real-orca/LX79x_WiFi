#pragma once
#include <Arduino.h>
#include "HW_Models.h"

/*
 * Mower Model
 */
//#define HW_MODEL LX790_V1_0
#define HW_MODEL LX790_V1_1

/*
 * Debug
 */
#define DEBUG_SERIAL_PRINT 1

/*
 * config structure
 */
typedef struct {
  bool captivePortal;
  String portalPassword;
  uint32_t portalTimeout;
  String wifiSSID;
  String wifiPassword;
  String hostname;
  char pin[5];
} LX790_Config;
extern LX790_Config config;
