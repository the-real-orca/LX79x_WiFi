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
#define DEBUG_SERIAL_PRINT 0 // 1: enable debug serial print, 0: disable debug serial print

#if DEBUG_SERIAL_PRINT
  #define DEBUG_println(x)  Serial.println(x)
  #define DEBUG_print(x)  Serial.print(x)
  #define DEBUG_printf(x, y)  Serial.printf(x, y)
#else  
  #define DEBUG_println(x)
  #define DEBUG_print(x)
  #define DEBUG_printf(x, y)
#endif

/*
 * config structure
 */
typedef struct {
  bool wifiEnabled;
  bool captivePortal;
  String portalPassword;
  uint32_t portalTimeout;
  String wifiSSID;
  String wifiPassword;
  String hostname;
  char pin[5];
} LX790_Config;
extern LX790_Config config;
