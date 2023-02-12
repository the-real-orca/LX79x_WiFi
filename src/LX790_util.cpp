#include <Arduino.h>
#include "LX790_util.h"
#include "HAL_LX790.h"


/*
 * segments:
 *
 *   -- 1 --
 *  |       |
 *  6       2
 *  |       |
 *   -- 7 --
 *  |       |
 *  5       3
 *  |       |
 *   -- 4 --
 */

#ifndef SEG1
  #define SEG1 0
  #define SEG2 0
  #define SEG3 0
  #define SEG4 0
  #define SEG5 0
  #define SEG6 0
  #define SEG7 0
#endif

struct
{
  const char c;
  const char pattern;
} const SegChr[] =
{
  {' ', 0x00},
  {'1', SEG2 | SEG3},
  {'2', SEG1 | SEG2 | SEG7 | SEG5 | SEG4 },
  {'3', SEG1 | SEG2 | SEG7 | SEG3 | SEG4 },
  {'4', SEG6 | SEG7 | SEG2 | SEG3 },
  {'5', SEG1 | SEG6 | SEG7 | SEG3 | SEG4 },
  {'6', SEG1 | SEG6 | SEG7 | SEG5 | SEG3 | SEG4 },
  {'7', SEG1 | SEG2 | SEG3 },
  {'8', SEG1 | SEG6 | SEG2 | SEG7 | SEG5 | SEG3 | SEG4 },
  {'0', SEG1 | SEG6 | SEG2 | SEG5 | SEG3 | SEG4 },
  {'9', SEG1 | SEG6 | SEG2 | SEG7 | SEG3 | SEG4}, 
  {'E', SEG1 | SEG6 | SEG7 | SEG5 | SEG4 },
  {'r', SEG7 | SEG5 },
  {'o', SEG7 | SEG3 | SEG4 | SEG5},                   // off is "0FF"
  {'F', SEG1 | SEG6 | SEG7 | SEG5},
  {'t', SEG6 | SEG7 | SEG5 | SEG4 },
  {'^', SEG1 },
  {'-', SEG7 },
  {'_', SEG4 },
  {'[', SEG1 | SEG6 | SEG5 | SEG4},
  {']', SEG1 | SEG2 | SEG3 | SEG4},
  {'=', SEG1 | SEG4},
  {'A', SEG1 | SEG6 | SEG2 | SEG7 | SEG5 | SEG3 },
  {'I', SEG3 | SEG2 },                               // !! wie '1'
  {'d', SEG2 | SEG7 | SEG5 | SEG3 | SEG4 },
  {'L', SEG6 | SEG5 | SEG4 },
  {'P', SEG1 | SEG6 | SEG2 | SEG7 | SEG5 },
  {'n', SEG5 | SEG7 | SEG3 },
  {'O', SEG1 | SEG6 | SEG2 | SEG5 | SEG3 | SEG4 },   // !! wie '0'
  {'U', SEG6 | SEG2 | SEG5 | SEG3 | SEG4},
  {'S', SEG1 | SEG6 | SEG7 | SEG3 | SEG4},           // !! wie '5'
  {'b', SEG6 | SEG7 | SEG5 | SEG3 | SEG4},
  {'H', SEG6 | SEG2 | SEG7 | SEG5 | SEG3 },
  {0, 0 }
};



/*****************************************************************************/
struct
{
  const char * Display;
  const LX790_Mode Mode;
  const char * Str;
} const LcdToMode[] =
{
  {"-F1-", LX790_RAIN,  "Regenverzögerung aktiviert."},
  {"-E1-", LX790_ERROR, "Der Robi befindet sich außerhalb des Funktionsbereichs."},
  {"-E2-", LX790_ERROR, "Radmotor blockiert."},
  {"-E3-", LX790_ERROR, "Messer blockiert."},
  {"-E4-", LX790_ERROR, "Der Robi steckt fest."},
  {"-E5-", LX790_ERROR, "Der Robi wird hochgehoben."},
  {"-E6-", LX790_ERROR, "Der Robi wird hochgehoben."},
  {"-E7-", LX790_ERROR, "Akkufehler"},
  {"-E8-", LX790_ERROR, "Es dauert zu lange, bis der Robi zur Ladestation zurückkehrt."},
  {"-EE-", LX790_ERROR, "Unbekannter Fehler."},
  {" OFF", LX790_OFF,   "ausgeschalten"},
  {"STOP", LX790_STOP,  "Gestoppt"},
  {"IDLE", LX790_READY, "Warte auf Start"},
  {"[  ]", LX790_DOCKED, "in Ladestation"},
  {"^^^^", LX790_BLOCKED, "Mähen... Hindernis..."},
  {"Pin1", LX790_SET_PIN, "neuen Pin eingeben"},
  {"Pin2", LX790_SET_PIN, "neuen Pin bestätigen"},
  {"A 50", LX790_SET_AREA, "Fläche einstellen (Di, Fr: 9:00-9:30)"},
  {"A100", LX790_SET_AREA, "Fläche einstellen (Di, Fr: 9:00-9:45)"},
  {"A150", LX790_SET_AREA, "Fläche einstellen (Mo, Mi, Fr: 9:00-9:45)"},
  {"A200", LX790_SET_AREA, "Fläche einstellen (Mo-Fr: 9:00-9:45)"},
  {"A300", LX790_SET_AREA, "Fläche einstellen (Mo-Fr: 9:00-10:00)"},
  {"A400", LX790_SET_AREA, "Fläche einstellen (Mo-Fr: 9:00-10:15)"},
  {"A500", LX790_SET_AREA, "Fläche einstellen (Mo-Fr: 9:00-10:45)"},
  {"A600", LX790_SET_AREA, "Fläche einstellen (Mo-Fr: 9:00-11:00)"},
  {" USB", LX790_USB,     "USB Stick erkannt"},
  {nullptr, LX790_UNKNOWN, ""}
};


/*
ER:50	Error updating main cpu firmware (.pck)
ER:51	Error opening .pck file
ER:52	Error firmware downgrade not allowed
EF:80	Error updating motor firmware (factory first program)
ER:80	Error updating motor firmware
ER:82	Error updating sensorMcu firmware (ultrasound/wire)
ER:83	Error updating imu (BOSCH BNO055)
ER:84	Error initializing imu (BOSCH BNO055)
*/

struct
{
  const char * Display;
  const char * Str;
} const SegmentToLetter[] =
{
  {"5toP", "STOP"},
  {"^^^^", "^^^^"},
  {"____", "^^^^"},
  {"[==]", "[  ]"},
  {"1dLE", "IDLE"},
  {"   -", "IDLE"},
  {"  -1", "IDLE"},
  {" -1d", "IDLE"},
  {"-1dL", "IDLE"},
  {"1dLE", "IDLE"},
  {"dLE-", "IDLE"},
  {"LE- ", "IDLE"},
  {"E-  ", "IDLE"},
  {"-   ", "IDLE"},
  {"   0", " OFF"},
  {"  0F", " OFF"},
  {" 0FF", " OFF"},
  {"0FF ", " OFF"},
  {"FF  ", " OFF"},
  {"F   ", " OFF"},
  {"P1n1", "Pin1"},
  {"P1n2", "Pin2"},
  {" U56", " USB"},
  {"U56 ", " USB"},
  {nullptr,""}
};

char decodeChar (char raw)
{
  int i = 0;
  
  for (i = 0; SegChr[i].c; i++)
  {
    if (SegChr[i].pattern == raw)
    {
      return SegChr[i].c;
    }
  }
  
  return '#';
}

uint8_t encodeSeg (uint8_t c)
{
  int i = 0;
  
  for (i = 0; SegChr[i].c; i++)
  {
    if (SegChr[i].c == c)
    {
      return SegChr[i].pattern;
    }
  }
  
  return (SEG1 | SEG4 | SEG7);
}

inline bool compareDigits(const char a[4], const char b[4]) {
  return memcmp(a,b,4) == 0;
}

void decodeDisplay(LX790_State &state) {
  static unsigned long lastModeUpdate = 0;
  unsigned long time = millis();
  unsigned long delta = time - lastModeUpdate;
  static bool unlockPin = true;
  state.msg = "";

  // process segments
  int segCnt=0;
  for (int i = 0; i<4; i++)
  {
    byte seg = state.segments[i];
    state.digits[i] = decodeChar(seg);
    seg = seg & (~SEG7); // clear '-' segment
    while (seg) {
      if (seg & 0x01)
        segCnt++; 
      seg = seg >> 1;
    }
  }

  // normalize digits (e.g. for scrolling text)
  for (int i = 0; SegmentToLetter[i].Display; i++)
  {
    if ( compareDigits(state.digits, SegmentToLetter[i].Display) )
    {
      memcpy(state.digits, SegmentToLetter[i].Str, 4);
      break;
    }
  }
 
  // mode
  if ( segCnt == 1 ) {
    // running
    state.mode = LX790_RUNNING;
    state.msg = "läuft ...";
    for (int i = 0; i<4; i++)
    {
      state.digits[i] = state.segments[i] ? ' ' : '*';
    }
  } else if ( compareDigits(state.digits, "8888") && state.point == ':' ) {
    state.mode = LX790_POWER_UP;
    unlockPin = true;
  } else if (state.lock==true && (state.digits[3]=='-' || delta < 5000)) {
    if ( state.mode == LX790_SET_PIN )
      state.mode = LX790_SET_PIN;
    else
      state.mode = LX790_ENTER_PIN;
    state.msg = "PIN eingeben";
    if ( state.digits[3]=='-' )
      lastModeUpdate = time; 
  } else if ( compareDigits(state.digits, "    ") ) {
    if ( (state.clock || state.battery) && state.mode != LX790_OFF ) {
      state.mode = LX790_STANDBY;
      state.msg = "standby";
    } else {
      state.mode = LX790_OFF;
      memcpy(state.digits, " OFF", 4);
      state.msg = "ausgeschalten";
    }
    unlockPin = true;
  } else if ( compareDigits(state.digits, "[  ]") ) {
    static uint8_t oldBattery = 0;
    if ( (state.battery > oldBattery) || delta < 5000 ) {
      state.mode = LX790_CHARGING;
      state.msg = "Laden ...";
       if (state.battery > oldBattery)
        lastModeUpdate = time; 
    } else {
      state.mode = LX790_DOCKED;
      state.msg = "in Ladestation";
    }
    oldBattery = state.battery;
  } else { // try to decode text
    for (int i = 0; LcdToMode[i].Display; i++)
    {
      if ( compareDigits(state.digits, LcdToMode[i].Display) )
      {
        state.msg = LcdToMode[i].Str;
        state.mode = LcdToMode[i].Mode;
        break;
      }
    }

  }

  #ifdef PIN
  if ( unlockPin && state.autoUnlock &&
        state.mode == LX790_ENTER_PIN && state.wifi) {
    // unlock robot if connected to WiFi
    static int8_t digitPos = 4;
    state.msg = "automatische Pin Eingabe";

    // set current digit position on start
    if ( digitPos >= 4 ) {
      if ( state.digits[0] == '-' ) {
        digitPos = 0;
      }
    }

    // key in current digit
    if ( digitPos < 4 && state.digits[digitPos] != '-' 
          && (uxQueueMessagesWaiting(cmdQueue) == 0) ) {
      if ( state.digits[digitPos] < PIN[digitPos] ) {
        // send '+' button
        queueButton(BTN_START);
      } else if ( state.digits[digitPos] > PIN[digitPos] ) {
        // send '-' button
        queueButton(BTN_HOME);
      } else {
        // send OK
        queueButton(BTN_OK);
        CMD_Type cmd = {CMD_Type::WAIT, 250}; xQueueSend(cmdQueue, &cmd, 0);        
        digitPos++;
        if ( digitPos==4 ) {
          unlockPin = false;
        }
      }
    }
  }
  #else
  if ( state.autoUnlock && state.mode == LX790_ENTER_PIN ) {
    // unlock robot if connected to WiFi
    state.msg = "Pin nicht definiert";
  }
  #endif

}

void queueButton(BUTTONS btn, int delay) {
  CMD_Type cmd;
  cmd = {CMD_Type::BTN_PRESS, btn};
  xQueueSend(cmdQueue, &cmd, 0);
  cmd = {CMD_Type::WAIT, delay};
  xQueueSend(cmdQueue, &cmd, 0);
  cmd = {CMD_Type::BTN_RELEASE, btn};
  xQueueSend(cmdQueue, &cmd, 0);
}
