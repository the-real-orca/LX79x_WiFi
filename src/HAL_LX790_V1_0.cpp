#include "config.h"
#ifdef HW_MODEL
#if HW_MODEL == LX790_V1_0

#include <Wire.h>
#include "CRC.h"
#include "ahWireSlave.h"
#include "LX790_util.h"

#include "HAL_LX790_V1_0.h"

static TwoWireSlave WireSlave  = TwoWireSlave(0); //ESP32 <-> Motherboard
static TwoWire      WireMaster = TwoWire(1);      //ESP32 <-> Display/Buttons
static int ProcInit = 1;
static unsigned long Lst_ButtonReqFromMainboard = 0;
static uint8_t DatReadBuff[LEN_MAINBOARD_MAX*5] = {0};
static int IdxReadBuff = 0;
static int ReadBuff_Processed = 0;
static uint8_t DatMainboard[LEN_MAINBOARD_MAX] = {0};
static uint8_t Lst_DatMainboard[LEN_MAINBOARD_MAX] = {0};
static uint8_t DisplayRes[LEN_DISPLAY_RES] = {0x01, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0xFB, 0xA9};

void HAL_setup()
{
  // init HW Communication
  memset(DatMainboard, 0, sizeof DatMainboard);
  pinMode(OUT_IO, OUTPUT);
  digitalWrite(OUT_IO, LOW);
  bool ret = WireSlave.begin(SDA_PIN_MAINBOARD, SCL_PIN_MAINBOARD, I2C_SLAVE_ADDR);
  if (!ret)
  {
    Serial.println(F("I2C slave init failed"));
    while(1);
  }
  WireMaster.begin(SDA_PIN_DISPLAY, SCL_PIN_DISPLAY, 100000UL);
}

void HAL_loop(LX790_State &state) {
  int err = 0;
  int ret;
  
  //after power up
  if (ProcInit && ( (millis() > 10*2000)             /*Timeout*/ ||
                      DatMainboard[LEN_DISPLAY_RES-1] /*Komm. ok*/ ) ) 
  {
    ProcInit = 0;
    digitalWrite(OUT_IO, HIGH);
  }
  
  //get state/response from buttons
  while (!ProcInit)
  {
    if (WireMaster.requestFrom(I2C_DISPLAY_ADDR, LEN_DISPLAY_RES) != LEN_DISPLAY_RES)
      break;
    ret = WireMaster.available();
    if (!ret)
      break;

    if (ret != LEN_DISPLAY_RES)
    {
      WireMaster.flush();
      Serial.print  (F("should never see me - L: "));
      Serial.println(__LINE__);
      break;
    }

    int i = 0;
    while (WireMaster.available())
      DisplayRes[i++] = WireMaster.read();

    break;
  }

  //read Mainboard data
  while (1)
  {
    memset(DatMainboard, 0, sizeof DatMainboard);

    ret = WireSlave.read_buff(&DatReadBuff[IdxReadBuff], (sizeof DatReadBuff)-IdxReadBuff);

    if (ret < 0)
    {
      err = __LINE__;
      break; //err driver
    }

    IdxReadBuff += ret;

    if ((DatReadBuff[0] == TYPE_BUTTONS) && (IdxReadBuff >= LEN_BUTTONS_REQ))
    {
      static uint8_t Req[] = {0x01, 0x01, 0xE0, 0xC1};
      if(!memcmp(DatReadBuff, Req, sizeof Req))
      {
        memcpy(DatMainboard, DatReadBuff, LEN_BUTTONS_REQ);
        ReadBuff_Processed = LEN_BUTTONS_REQ;
      }
    }
    else if ((DatReadBuff[0] == TYPE_UNKNOWN) && (IdxReadBuff >= LEN_UNKNOWN_REQ))
    {
      static uint8_t Req[] = {0x04, 0x01, 0x15, 0x3E};
      if(!memcmp(DatReadBuff, Req, sizeof Req))
      {
        memcpy(DatMainboard, DatReadBuff, LEN_UNKNOWN_REQ);
        ReadBuff_Processed = LEN_UNKNOWN_REQ;
      }
    }
    else if ((DatReadBuff[0] == TYPE_UNKNOWN_INIT) && (IdxReadBuff >= LEN_UNKNOWN_INIT_REQ))
    {
      static uint8_t Req[] = {0x05, 0x01, 0x01, 0x83, 0xfb};
      if(!memcmp(DatReadBuff, Req, sizeof Req))
      {
        memcpy(DatMainboard, DatReadBuff, LEN_UNKNOWN_INIT_REQ);
        ReadBuff_Processed = LEN_UNKNOWN_INIT_REQ;
      }
    }
    else if (DatReadBuff[0] == TYPE_DISPLAY && IdxReadBuff >= LEN_DISPLAY_RES)
    {
      uint16_t calc_crc = 0xFFFF;
      uint16_t msg_crc = 0x0000;

      //crc.restart();
      //crc.add(&DatReadBuff[ReadBuff_Processed], LEN_DISPLAY_RES-2); -> yield() -> error !! -> ESP32 bug ??
      //calc_crc = crc.getCRC();
      calc_crc = crc16(DatReadBuff, LEN_DISPLAY_RES-2, 0x1021, 0xFFFF, 0xFFFF, false, false);

      msg_crc |= (DatReadBuff[LEN_DISPLAY_RES-2]);
      msg_crc |= (DatReadBuff[LEN_DISPLAY_RES-1])<<8;
      if (calc_crc != msg_crc)
      {
        err = __LINE__;
        break; //invalid crc
      }
      else
      {
        memcpy(DatMainboard, DatReadBuff, LEN_DISPLAY_RES);
        ReadBuff_Processed = LEN_DISPLAY_RES;
      }
    }

    if (ReadBuff_Processed)
    {
      if (ret == 0)
        delay(5);
    
      memcpy(DatReadBuff, &DatReadBuff[ReadBuff_Processed], (sizeof DatReadBuff)-ReadBuff_Processed);
      IdxReadBuff -= ReadBuff_Processed;
    }
    else
    {
      if (IdxReadBuff >= LEN_MAINBOARD_MAX*3)
      {
        err = __LINE__;
        break; //no match
      }
    }
    ReadBuff_Processed = 0;

    break;
  }

  //valid data from mainboard?
  if (err)
  {   
    #if(DEBUG_SERIAL_PRINT  == 1)
    {
      int i = 0;
      char hex[2] = {0};
      char buff[(sizeof DatReadBuff)*2 + 1] = {0};

      Serial.print  ("Err Slave MB ret: ");
      Serial.print  (ret, DEC);
      Serial.print  (" err: ");
      Serial.print  (err, DEC);
      Serial.print  (" Data Read: ");
      memset(buff, 0, sizeof buff);
      for (i=0; i<(sizeof DatReadBuff); i++)
      {
        sprintf(hex, "%02x", DatReadBuff[i]);
        strcat(buff, hex);
      }
      Serial.print(buff);

      Serial.print (" Data MB: ");
      memset(buff, 0, sizeof buff);
      for (i=0; i<(sizeof DatMainboard); i++)
      {
        sprintf(hex, "%02x", DatMainboard[i]);
        strcat(buff, hex);
      }
      Serial.print(buff);
      Serial.println(" ");
    }
    #endif

    xSemaphoreTake(SemMutex, 1);
    thExchange.Lst_err = err;
    thExchange.Cnt_err++;
    xSemaphoreGive(SemMutex);
    WireSlave.flush();
    IdxReadBuff = ReadBuff_Processed = 0;
    memset(DatReadBuff, 0, sizeof DatReadBuff);
  }
  
  // show start-up screen
  // @TODO move to write section
  if (ProcInit && !(millis()%100))
  {
    uint8_t InitDisplay[LEN_MAINBOARD_MAX] = {0};
    char num[4] = {0};
    uint16_t calc_crc = 0xFFFF;

    sprintf(num, "%03lu", millis()/100);

    InitDisplay[0] = TYPE_DISPLAY;
    InitDisplay[1] = EncodeSeg('P');
    InitDisplay[2] = EncodeSeg((uint8_t)num[0]);
    InitDisplay[3] = EncodeSeg((uint8_t)num[1]);
    InitDisplay[4] = EncodeSeg((uint8_t)num[2]);
    InitDisplay[5] = (WiFi.status() == WL_CONNECTED)?0x10:0;  //WiFi Symbol
    InitDisplay[6] = 0xC8;

    calc_crc = crc16(InitDisplay, LEN_DISPLAY_RES-2, 0x1021, 0xFFFF, 0xFFFF, false, false);
    InitDisplay[LEN_DISPLAY_RES-2] = calc_crc & 0xff;
    InitDisplay[LEN_DISPLAY_RES-1] = calc_crc>>8;

    WireMaster.beginTransmission(I2C_DISPLAY_ADDR);
    WireMaster.write(InitDisplay, LEN_DISPLAY_RES);
    WireMaster.endTransmission(true);
  }

  // copy to LCD via I2C
  // @TODO move to write section
  if (DatMainboard[0])
  {
    size_t size = 0;
    
    Lst_ButtonReqFromMainboard = millis();

    switch (DatMainboard[0])
    {
      //case TYPE_DISPLAY:
      //  size = LEN_DISPLAY_RES;
      //  break;
      case TYPE_BUTTONS:
        size = LEN_BUTTONS_REQ;
        break;
      case LEN_UNKNOWN_REQ:      //04 01 15 3E
        size = 4;
        break;
      case LEN_UNKNOWN_INIT_REQ: //05 01 01 83 fb
        size = 5;
        break;
    }
    
    if (size)
    {
      WireMaster.beginTransmission(I2C_DISPLAY_ADDR);
      WireMaster.write(DatMainboard, size);
      WireMaster.endTransmission(true);
    }
  }
  
  //Timeout or off?
  if (millis() - Lst_ButtonReqFromMainboard > 100)
  {
    xSemaphoreTake(SemMutex, 1);
    thExchange.Cnt_timeout++;
    memset(&thExchange.WebInButtonTime, 0, sizeof thExchange.WebInButtonTime);
    memset(&thExchange.WebInButtonState, 0, sizeof thExchange.WebInButtonState);
    if (thExchange.cmdQueIdx)
    { 
      memset(thExchange.cmdQue, 0, sizeof thExchange.cmdQue);
      thExchange.cmdQueIdx = 0;
    }
    xSemaphoreGive(SemMutex);
    Lst_ButtonReqFromMainboard = millis();

    WireMaster.flush();
    #if(DEBUG_SERIAL_PRINT  == 1)
    {
      int i = 0;
      char hex[2] = {0};
      char buff[(sizeof DatReadBuff)*2 + 1] = {0};
      int NotEmpty = 0;

      memset(buff, 0, sizeof buff);
      for (i=0; i<(sizeof DatReadBuff); i++)
      {
        sprintf(hex, "%02x", DatReadBuff[i]);
        strcat(buff, hex);
        if (DatReadBuff[i])
          NotEmpty = 1;
      }
      if (NotEmpty)
      {
        Serial.print  (" To read: ");
        Serial.print  (IdxReadBuff);
        Serial.print  (" proc: ");
        Serial.print  (ReadBuff_Processed);
        Serial.print  (" dat: ");
        Serial.println(buff);
      }
    }
    #endif

    WireSlave.flush();
    IdxReadBuff = ReadBuff_Processed = 0;
    memset(DatReadBuff, 0, sizeof DatReadBuff);
    
    DatMainboard[0] = TYPE_DISPLAY; //force for web counter
  }

  // inject button commands
  // @TODO move to write section
  if (DatMainboard[0] == TYPE_BUTTONS)
  {
    //Inject
    int t = 0;
    uint8_t WebInButton[2 /*byte 1 + byte 2*/] = {0};
    static int LstProcesedcmdQueIdx = 0;
    
    memset(WebInButton, 0, sizeof WebInButton);
    
    xSemaphoreTake(SemMutex, 1);
    
    //Buttons/Actions from que
    if (!thExchange.cmdQueIdx)
      LstProcesedcmdQueIdx = 0;
    while (thExchange.cmdQueIdx)
    {
      unsigned long AktTime = millis();

      if (thExchange.cmdQue[LstProcesedcmdQueIdx].T_end < AktTime)
      {
        LstProcesedcmdQueIdx++;
      }
      if ( (thExchange.cmdQueIdx >= LEN_CMDQUE) ||
          (!thExchange.cmdQue[LstProcesedcmdQueIdx].WebInButton[0] &&
            !thExchange.cmdQue[LstProcesedcmdQueIdx].WebInButton[1]))
      {
        memset(thExchange.cmdQue, 0, sizeof thExchange.cmdQue);
        thExchange.cmdQueIdx = 0;
        break;
      }
      if (thExchange.cmdQue[LstProcesedcmdQueIdx].T_start < AktTime)
      {
        WebInButton[0] = thExchange.cmdQue[LstProcesedcmdQueIdx].WebInButton[0];
        WebInButton[1] = thExchange.cmdQue[LstProcesedcmdQueIdx].WebInButton[1];
      }
      
      break;
    }
    
    //Buttons from web
    for (t=0; Buttons[t]; t++)
    {
      if ( thExchange.WebInButtonState[t] ||
            thExchange.WebInButtonTime[t] > millis() )
      {
        if (t == 0 /*"io"*/)
          WebInButton[0] |= 0;
        if (t == 1 /*"start"*/)
          WebInButton[0] |= BTN_BYTE1_START;
        if (t == 2 /*"home"*/)
          WebInButton[0] |= BTN_BYTE1_HOME;
        if (t == 3 /*"ok"*/)
          WebInButton[0] |= BTN_BYTE1_OK;
        if (t == 4 /*"stop"*/)
          WebInButton[1] |= BTN_BYTE2_STOP;
      }
      else
      {
        thExchange.WebInButtonTime[t] = 0;
      }
    }
    xSemaphoreGive(SemMutex);
          
    if (WebInButton[0] || WebInButton[1])
    {
      uint16_t calc_crc = 0xFFFF;
      
      //01 02 78 00 00 00 00 BB 22
      //|| || -- Button: stop
      //|| -- Buttons: home, start, ok
      //-- Type

      if (WebInButton[0])
        DisplayRes[1] = WebInButton[0];
      if (WebInButton[1])
        DisplayRes[2] = WebInButton[1];

      //crc.restart();
      //crc.add(DisplayRes, LEN_DISPLAY_RES-2); -> yield() -> error !! -> ESP32 bug ??
      //calc_crc = crc.getCRC();
      calc_crc = crc16(DisplayRes, LEN_DISPLAY_RES-2, 0x1021, 0xFFFF, 0xFFFF, false, false);

      DisplayRes[LEN_DISPLAY_RES-2] = calc_crc & 0xff;
      DisplayRes[LEN_DISPLAY_RES-1] = calc_crc>>8;
    }

    ret = WireSlave.write_buff(DisplayRes, LEN_DISPLAY_RES);
    if (ret < 0)
    {
      Serial.print  (F("Ret write "));
      Serial.println(ret, DEC);
    }
  }
  // write to LCD via I2C
  else if (DatMainboard[0] == TYPE_DISPLAY)
  {
    static unsigned long Lst_bat_charge = 0;
    
    // @TODO move to write section
    if (DatMainboard[LEN_DISPLAY_RES-1]) //valid or just forced from timeout
    {
      WireMaster.beginTransmission(I2C_DISPLAY_ADDR);
      WireMaster.write(DatMainboard, LEN_DISPLAY_RES);
      WireMaster.endTransmission(true);
    }

    // decode raw display data
    if ( memcmp(Lst_DatMainboard, DatMainboard, sizeof Lst_DatMainboard) ||
        (Lst_bat_charge && (millis() - Lst_bat_charge > 1000)) )
    {
      static unsigned int CntWebOut = 0;

      memcpy(Lst_DatMainboard, DatMainboard, sizeof Lst_DatMainboard);
      CntWebOut++;

      // LCD digits
      memcpy(state.segments, DatMainboard, 4);
      state.point = ' ';
      if (DatMainboard[5] & 0x01)
        state.point = '.';
      if (DatMainboard[5] & 0x02)
        state.point = '\'';
      if ((DatMainboard[5] & 0x01) && (DatMainboard[5] & 0x02))
        state.point = '.';
      state.brightness = 15;

      //battery
      uint8_t batraw = DatMainboard[5] & 0xE0;
      if (DatMainboard[6] & 0x01)
        state.battery = 3;    //"full"
      else if (batraw == 0xE0)
        state.battery = 2;    //"mid"
      else if (batraw == 0x60)
        state.battery = 1;    //"low"
      else
        state.battery = 0;    //"empty" (box only)

      // clock      
      state.clock = (DatMainboard[5] & 0x04)?1:0;

      // lock
      state.lock = (DatMainboard[5] & 0x08)?1:0;

      // wifi      
      state.wifi = (DatMainboard[5] & 0x10)?1:0;

    }
  }

}


#endif
#endif