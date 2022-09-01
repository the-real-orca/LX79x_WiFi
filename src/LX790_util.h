#pragma once
#include <stdint.h>

typedef enum {LX790_UNKNOWN = 0, LX790_POWER_UP, LX790_POWER_DOWN, LX790_ENTER_PIN, LX790_READY, LX790_RUNNING, LX790_BLOCKED, LX790_GOING_HOME, LX790_STOP, LX790_CHARGING, LX790_SLEEP,  LX790_OFF, LX790_SET_PIN, LX790_SET_DATE, LX790_SET_TIME, LX790_SET_AREA, LX790_RAIN, LX790_ERROR=-1} LX790_Mode;
//typedef enum {LX790_UNKNOWN = 0} LX790_Mode;

typedef struct {
  byte segments[4];
  char digits[4];
  char point;		// '.' or '\'' or ':'
  byte clock;
  byte wifi;
  byte lock;
  byte battery;
  byte brightness;
  LX790_Mode mode;
  const char *msg;
} LX790_State;

char decodeChar (char raw);
uint8_t encodeSeg (uint8_t c);
void decodeDisplay(LX790_State &out);

#include <Arduino.h>

extern SemaphoreHandle_t SemMutex;


