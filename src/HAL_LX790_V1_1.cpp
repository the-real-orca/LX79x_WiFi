#include "config.h"
#ifdef HW_MODEL
#if HW_MODEL == LX790_V1_1

#include <ESP32SPISlave.h>
#include "LX790_util.h"

#include "HAL_LX790_V1_1.h"

static inline byte getButtonPin(BUTTONS btn) {
  switch (btn) {
    case BTN_IO:
      return BTN_PIN_IO;
    case BTN_START:
      return BTN_PIN_START;
    case BTN_HOME:
      return BTN_PIN_HOME;
    case BTN_OK:
      return BTN_PIN_OK;
    case BTN_STOP:
      return BTN_PIN_STOP;
    default:
      return 0;
  }  
}

void HAL_buttonPress(BUTTONS btn) {
  byte pin = getButtonPin(btn);
  if ( btn == BTN_STOP ) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 1);
  } else {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
  }
  DEBUG_printf("button pressed (D%d)\n", pin);
}

void HAL_buttonRelease(BUTTONS btn) {
  byte pin = getButtonPin(btn);
  if ( btn == BTN_STOP ) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
  } else {
    pinMode(pin, INPUT);
  }
  DEBUG_printf("button released (D%d)\n", pin);
}

bool decodeTM1668(const uint8_t raw[14], LX790_State &state) {
  byte val;


  // LCD digits
  uint16_t segCnt = 0;
  bool error = false;
  for (int i=0; i<4; i++) {
    byte segments = 0;
    byte mask = 1 << i;
    for (int j=0; j<7; j++) {
      segments |= (raw[j*2] & mask) ? (1<<j) : 0;
    }
    if ( state.segments[i] != segments ) state.updated = true;
    state.segments[i] = segments;
    byte seg = segments;
    while (seg) {
      if (seg & 0x01)
        segCnt++; 
      seg = seg >> 1;
    }

    if ( decodeChar(segments) == '#' ) {
      // error reading display
      error = true;
    }
  }
  // return if error on decoding digits, if exactly 1 segment active -> running (no error)
  if ( error && segCnt != 1 )
    return false;

  for (int i=0; i<4; i++) {
    state.digits[i] = decodeChar(state.segments[i]);
  }

  // clock
  val = bitRead(raw[0*2], 4) | bitRead(raw[1*2], 4);
  if ( state.clock != val ) state.updated = true;
  state.clock = val;

  // wifi
  // wifi is output / state.wifi = bitRead(raw[2*2], 4) | bitRead(raw[3*2], 4);

  // lock
  val = bitRead(raw[4*2], 4) | bitRead(raw[5*2], 4);
  if ( state.lock != val ) state.updated = true;
  state.lock = val;

  // battery
  val = bitRead(raw[2*2], 6) + bitRead(raw[1*2], 6) + bitRead(raw[0*2], 6);
  if ( state.battery != val ) state.updated = true;
  state.battery = val;

  // dots
  val = ' ';
  if ( bitRead(raw[3*2], 6) )
      val = '\'';
  if ( bitRead(raw[4*2], 6) )
      val = '.';
  if ( bitRead(raw[3*2], 6) && bitRead(raw[4*2], 6) )
      val = ':';
  if ( state.point != val ) state.updated = true;
  state.point = val;

  return true;
}

ESP32SPISlave tm1668;
uint8_t spi_slave_rx_buf[32];

void HAL_setup()
{
  // init HW Communication
  tm1668.setDataMode(SPI_MODE1);
  tm1668.setSlaveFlags(SPI_SLAVE_BIT_LSBFIRST);
  tm1668.begin(VSPI, CLK_PIN_DISPLAY, VSPI_MISO, DIO_PIN_DISPLAY, CS_PIN_DISPLAY);
//  tm1668.begin(HSPI);

  pinMode(CS_PIN_DISPLAY, INPUT);
  pinMode(DIO_PIN_DISPLAY, INPUT);
  pinMode(CLK_PIN_DISPLAY, INPUT);

  digitalWrite(BTN_PIN_IO, 0);
  digitalWrite(BTN_PIN_START, 0);
  digitalWrite(BTN_PIN_HOME, 0);
  digitalWrite(BTN_PIN_OK, 0);
  digitalWrite(BTN_PIN_STOP, 0);
  pinMode(BTN_PIN_IO, INPUT);
  pinMode(BTN_PIN_START, INPUT);
  pinMode(BTN_PIN_HOME, INPUT);
  pinMode(BTN_PIN_OK, INPUT);
  pinMode(BTN_PIN_STOP, OUTPUT);
}

void HAL_loop(LX790_State &state) {
  static LX790_State oldState = {0};
  static long btnHomeTime = -1;
  static byte btnHomeVal = -1;

  // read data via SPI
  if (tm1668.remained() == 0)
    tm1668.queue(spi_slave_rx_buf, sizeof spi_slave_rx_buf);

  // LCD
  while (tm1668.available()) {
    int size = tm1668.size();
    if ( size ) {
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
          if ( size == 15) {
            decodeTM1668(spi_slave_rx_buf+1, state);
          }
          break;
      }
    }
    tm1668.pop();
  }

  // press "HOME" button for 10 seconds to start captive portal
  unsigned long time = millis();
  byte pin = getButtonPin(BTN_HOME);
  byte val = digitalRead(pin);
  if ( btnHomeVal != val ) {
    DEBUG_print("button home: "); DEBUG_println(val);
    btnHomeVal = val;
    btnHomeTime = time;
  }
  
  if ( !config.captivePortal && state.brightness> 0 && btnHomeVal == 0 && ( (time-btnHomeTime) > 9000) ) {
    btnHomeTime = time; // reset timer for home botton to prevent multiple execution
    DEBUG_println("home button pressed 10s -> activate captive portal");
    CMD_Type xcmd;
    xcmd = {CMD_Type::DISCONNECT, 0}; xQueueSend(cmdQueue, &xcmd, 0);
    xcmd = {CMD_Type::WAIT, 1000}; xQueueSend(cmdQueue, &xcmd, 0);
    xcmd = {CMD_Type::WIFI_PORTAL, 0}; xQueueSend(cmdQueue, &xcmd, 0);
  }

}


#endif
#endif
