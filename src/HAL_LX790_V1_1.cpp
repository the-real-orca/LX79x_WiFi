#include "config.h"
#if HW_MODEL == LX790_V1_1

#include <Wire.h>
#include "CRC.h"
#include "ahWireSlave.h"
#include "LX790_util.h"

#include "HAL_LX790_V1_1.h"

void decodeTM1668(const uint8_t raw[7], LX790_State &state) {

  // clock
  state.clock = bitRead(raw[0], 4) | bitRead(raw[1], 4);

  // wifi
  state.wifi = bitRead(raw[2], 4) | bitRead(raw[3], 4);

  // lock
  state.lock = bitRead(raw[4], 4) | bitRead(raw[5], 4);

  // battery
  state.battery = bitRead(raw[2], 6) + bitRead(raw[1], 6) + bitRead(raw[0], 6);

  // dots
  state.point = ' ';
  if ( bitRead(raw[3], 6) )
      state.point = '\'';
  if ( bitRead(raw[3], 6) )
      state.point = '.';
  if ( bitRead(raw[3], 6) && bitRead(raw[3], 6) )
      state.point = ':';

  // LCD digits
  for (int i=0; i<4; i++) {
    byte segments = 0;
    byte mask = 1 << i;
    for (int j=0; j<7; j++) {
      segments |= (raw[j] & mask) ? (1<<j) : 0;
    }
    state.segments[i] = segments;
  }
}

static uint8_t rawData[7];

void HAL_setup()
{
  // init HW Communication

}

void HAL_loop(LX790_State &state) {

  // @TODO read data via SPI

  decodeTM1668(rawData, state);
}


#endif