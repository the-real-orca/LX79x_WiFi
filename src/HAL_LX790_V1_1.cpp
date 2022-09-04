#include "config.h"
#if HW_MODEL == LX790_V1_1

#include <ESP32SPISlave.h>
#include "LX790_util.h"

#include "HAL_LX790_V1_1.h"

void decodeTM1668(const uint8_t raw[14], LX790_State &state) {

  // clock
  state.clock = bitRead(raw[0*2], 4) | bitRead(raw[1*2], 4);

  // wifi
  state.wifi = bitRead(raw[2*2], 4) | bitRead(raw[3*2], 4);

  // lock
  state.lock = bitRead(raw[4*2], 4) | bitRead(raw[5*2], 4);

  // battery
  state.battery = bitRead(raw[2*2], 6) + bitRead(raw[1*2], 6) + bitRead(raw[0*2], 6);

  // dots
  state.point = ' ';
  if ( bitRead(raw[3*2], 6) )
      state.point = '\'';
  if ( bitRead(raw[3*2], 6) )
      state.point = '.';
  if ( bitRead(raw[3*2], 6) && bitRead(raw[3*2], 6) )
      state.point = ':';

  // LCD digits
  for (int i=0; i<4; i++) {
    byte segments = 0;
    byte mask = 1 << i;
    for (int j=0; j<7; j++) {
      segments |= (raw[j*2] & mask) ? (1<<j) : 0;
    }
    state.segments[i] = segments;
  }
}

ESP32SPISlave tm1668;
uint8_t spi_slave_rx_buf[32];
#if DEBUG_SERIAL_PRINT
  static uint8_t rawData[8][32];
  static uint8_t bufNr;
#endif

void HAL_setup()
{
#if DEBUG_SERIAL_PRINT
  bufNr=0;
#endif  

  // init HW Communication
  tm1668.setDataMode(SPI_MODE1);
  tm1668.setSlaveFlags(SPI_SLAVE_BIT_LSBFIRST);
  tm1668.begin(HSPI);

  pinMode(CS_PIN_DISPLAY, INPUT);
  pinMode(DIO_PIN_DISPLAY, INPUT);
  pinMode(CLK_PIN_DISPLAY, INPUT);

}

void HAL_loop(LX790_State &state) {
  bool printout = false;

  // read data via SPI
  if (tm1668.remained() == 0)
    tm1668.queue(spi_slave_rx_buf, sizeof spi_slave_rx_buf);

  while (tm1668.available()) {

    int size = tm1668.size();
    if ( !size ) {
      tm1668.pop();
      continue;
    }
      
#if DEBUG_SERIAL_PRINT
    memset(spi_slave_rx_buf+size, 0x00, 32-size);
    memcpy(rawData[bufNr++], spi_slave_rx_buf, 32);
#endif

    uint8_t cmd = spi_slave_rx_buf[0];
    switch ( cmd & DISPLAY_CMD_MASK ) {
      case DISPLAY_CMD_MODE_SET:
      case DISPLAY_CMD_DATA_SET:
        // ignore
        break;
      
      case DISPLAY_CMD_CONTROL:
        state.brightness = bitRead(cmd, 4) ? 0 : (cmd & 0xE) + 1;
        break;

      case DISPLAY_CMD_ADDRESS:
        if ( size == 15)
          decodeTM1668(spi_slave_rx_buf+1, state);
        printout = true;
        break;
    }

    tm1668.pop();
  }

  if ( printout ) {
#if DEBUG_SERIAL_PRINT
    for (int j = 0; j < bufNr; j++) {
      Serial.print("raw ");
      for (size_t i = 0; i < 32; ++i) {
          Serial.print(rawData[j][i], HEX);Serial.print(" ");
      }
      Serial.println();
    }
    bufNr = 0;
#endif

    Serial.print(" clock "); Serial.print(state.clock);
    Serial.print(" wifi "); Serial.print(state.wifi);
    Serial.print(" lock "); Serial.print(state.lock);
    Serial.print(" battery "); Serial.print(state.battery);
    Serial.print(" brightness "); Serial.print(state.brightness);
    Serial.print(" mode "); Serial.print(state.mode);
    Serial.print(" | LCD "); 
    Serial.print(state.digits[0]); Serial.print(state.digits[1]); Serial.print(state.point); Serial.print(state.digits[2]); Serial.print(state.digits[3]);
    Serial.println();
  }

  delay(1);
}


#endif