#include <Arduino.h>
#include "LX790_util.h"
#include "HAL_LX790.h"

/*
// mutex and shared memory
std::mutex stateMutex;
LX790_State stateShared;
std::mutex cmdMutex;
*/

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
  {" OFF", LX790_OFF,   "Ausgeschalten"},
  {"STOP", LX790_STOP,  "Gestoppt"},
  {"IDLE", LX790_READY, "Warte auf Start"},
  {"****", LX790_RUNNING, "Mähen..."},
  {"----", LX790_BLOCKED, "Mähen... Hindernis..."},
  {"Pin1", LX790_SET_PIN, "neuen Pin eingeben"},
  {"Pin2", LX790_SET_PIN, "neuen Pin bestätigen"},
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
  {"^^^^", "----"}, // blocked
  {"____", "----"}, // blocked
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
  {"[==]", "IDLE"},
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
  state.msg = "";
  
  // process segments
  int cnt=0;
  for (int i = 0; i<4; i++)
  {
    byte seg = state.segments[i];
    state.digits[i] = decodeChar(seg);
    while (seg) {
      if (seg & 0x01)
        cnt++; 
      seg = seg >> 1;
    }
  }
  if ( cnt == 1 ) {
    memcpy(state.digits, "****", 4);  // running
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
  if ( compareDigits(state.digits, "8888") && state.point == ':' ) {
    state.mode = LX790_POWER_UP;
  } else if ( state.mode == LX790_POWER_UP && state.digits[3]=='-') {
    state.mode = LX790_ENTER_PIN;
  } else if ( compareDigits(state.digits, "    ") ) {
    if ( state.clock || state.battery )
      state.mode = LX790_SLEEP;
    else
      state.mode = LX790_OFF;
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

}

