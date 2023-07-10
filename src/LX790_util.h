#pragma once
#include <stdint.h>
#include <mutex>

typedef enum {LX790_UNKNOWN = 0, LX790_POWER_UP, LX790_ENTER_PIN, LX790_READY, 
        LX790_RUNNING, LX790_BLOCKED, LX790_GOING_HOME, LX790_STOP, LX790_CHARGING, 
        LX790_DOCKED, LX790_STANDBY, LX790_OFF, LX790_USB, 
        LX790_SET_PIN, LX790_SET_DATE, LX790_SET_TIME, LX790_SET_AREA, 
        LX790_RAIN, LX790_ERROR} LX790_Mode;
static const char* ModeNames[] = {"LX790_UNKNOWN", "LX790_POWER_UP", "LX790_ENTER_PIN", "LX790_READY",
        "LX790_RUNNING", "LX790_BLOCKED", "LX790_GOING_HOME", "LX790_STOP", "LX790_CHARGING",
        "LX790_DOCKED", "LX790_STANDBY", "LX790_OFF", "LX790_USB",
        "LX790_SET_PIN", "LX790_SET_DATE", "LX790_SET_TIME", "LX790_SET_AREA",
        "LX790_RAIN", "LX790_ERROR"};

#define EEPROM_debugLog 0
#define EEPROM_autoUnlock 1
#define EEPROM_SIZE 2

typedef struct {
  uint8_t segments[4];
  char digits[4];
  char point;		// '.' or '\'' or ':'
  uint8_t clock;
  uint8_t wifi;
  uint8_t lock;
  uint8_t battery;
  uint8_t brightness;
  LX790_Mode mode;
  LX790_Mode detectedMode;  
  bool updated;
  bool autoUnlock;
  bool debugLog;
  String hostname;
  String msg;
} LX790_State;

// LX790 State TaskHW --> TaskWeb
extern QueueHandle_t stateQueue;


typedef enum {BTN_NA = 0, BTN_IO, BTN_START, BTN_HOME, BTN_OK, BTN_STOP} BUTTONS;
static const char* ButtonNames[] = {"n/a", "io", "start", "home", "ok", "stop", nullptr};
typedef struct {
  enum {NA = 0, BTN_PRESS, BTN_RELEASE, WAIT, REBOOT, AUTOUNLOCK, DEBUGLOG, DISCONNECT, WIFI_CLIENT, WIFI_PORTAL} cmd;
  signed int param;
} CMD_Type;
// LX790 Commands TaskWeb --> TaskHW
extern QueueHandle_t cmdQueue;
void queueButton(BUTTONS btn, int delay = 250);

// display helper functions
char decodeChar (char raw);
uint8_t encodeSeg (uint8_t c);
void decodeDisplay(LX790_State &out);
