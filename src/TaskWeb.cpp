#include <Arduino.h>
#include <string.h>
#include <Wire.h>
//#include "CRC16.h"
#include "CRC.h"
#include <WiFi.h>
#include <WebServer.h>
#include "SPIFFS.h"
#include <Update.h>
#include "LX790_util.h"

// Copy "config_sample.h" to "config.h" and change it according to your setup.
#include "config.h"

WebServer server(80);
LX790_State state;

// Request:   http://MOWERADRESS/web
// Response:  [cnt];[Display];[point];[lock];[clock];[bat];[rssi dbm];[Cnt_timeout];[Cnt_err];[LstError];[MowerStatustext]
void Web_aktStatusWeb()
{
  char out[400] = "";
  
  sprintf(out, "%s;%d;%d;%d;%d;%s", 
    state.digits,
    WiFi.RSSI(), 
    0, // Cnt_timeout, 
    0, // Cnt_err, 
    0, // Lst_err,
    state.msg);
    
  server.send(200,"text/plain", out);
}

// Request:   http://MOWERADRESS/statval
// Response:  [DisplayWithDelimiter];[rssi dbm];[batAsText];[MowerStatustext]
void Web_aktStatusValues()
{
  char out[200] = "";
  const char* BatState[] = {"off", "empty", "low", "mid", "full", "charging"};
  int IdxBatState = 0;
  
  if (state.mode == LX790_CHARGING)
    IdxBatState = 5; /*charging*/
  else
    IdxBatState = state.battery + 1;

  sprintf(out, "%c%c%c%c%c;%d;%s;%s",
          state.digits[0],
          state.digits[1],
          state.point,
          state.digits[2],
          state.digits[3],
          WiFi.RSSI(),
          BatState[IdxBatState],
          state.msg);
  
  server.send(200,"text/plain", out);
}

//Webcommand examples: 
// Send command:         http://MOWERADRESS/cmd?parm=[command/button]&value=[state/time]
// Send command example: http://MOWERADRESS/cmd?parm=start&value=1
void Web_getCmd()
{
  if (server.argName(0) == "parm" &&
      server.argName(1) == "value")
  {
    int i = 0;
    int val = server.arg(1).toInt();
    
/* @TODO    
    xSemaphoreTake(SemMutex, 1);

    if (thExchange.cmdQueIdx)
    {
      server.send(500, "text/plain", "busy...");
      return;
    }

    if (server.arg(0) == "workzone" && val > 0)
    {
      thExchange.cmdQue[thExchange.cmdQueIdx].T_start = millis();
      thExchange.cmdQue[thExchange.cmdQueIdx].T_end = millis()+3500;
      thExchange.cmdQue[thExchange.cmdQueIdx].WebInButton[0] = BTN_BYTE1_OK;
      thExchange.cmdQueIdx++;
    }
    else if (server.arg(0) == "timedate" && val > 0)
    {
      thExchange.cmdQue[thExchange.cmdQueIdx].T_start = millis();
      thExchange.cmdQue[thExchange.cmdQueIdx].T_end = millis()+3500;
      thExchange.cmdQue[thExchange.cmdQueIdx].WebInButton[0] = BTN_BYTE1_START;
      thExchange.cmdQueIdx++;
    }
    else if (server.arg(0) == "startmow" && val > 0)
    {
      thExchange.cmdQue[thExchange.cmdQueIdx].T_start = millis();
      thExchange.cmdQue[thExchange.cmdQueIdx].T_end = millis()+200;
      thExchange.cmdQue[thExchange.cmdQueIdx].WebInButton[0] = BTN_BYTE1_START;
      thExchange.cmdQueIdx++;
      thExchange.cmdQue[thExchange.cmdQueIdx].T_start = millis()+300;
      thExchange.cmdQue[thExchange.cmdQueIdx].T_end = millis()+500;
      thExchange.cmdQue[thExchange.cmdQueIdx].WebInButton[0] = BTN_BYTE1_OK;
      thExchange.cmdQueIdx++;
    }
    else if (server.arg(0) == "homemow" && val > 0)
    {
      thExchange.cmdQue[thExchange.cmdQueIdx].T_start = millis();
      thExchange.cmdQue[thExchange.cmdQueIdx].T_end = millis()+200;
      thExchange.cmdQue[thExchange.cmdQueIdx].WebInButton[0] = BTN_BYTE1_HOME;
      thExchange.cmdQueIdx++;
      thExchange.cmdQue[thExchange.cmdQueIdx].T_start = millis()+300;
      thExchange.cmdQue[thExchange.cmdQueIdx].T_end = millis()+500;
      thExchange.cmdQue[thExchange.cmdQueIdx].WebInButton[0] = BTN_BYTE1_OK;
      thExchange.cmdQueIdx++;
    }
    else
    {
      for (i=0; Buttons[i]; i++)
      {
        if (server.arg(0) == Buttons[i])
        {
          if (i==0) //OnOff pushbutton
          {
            digitalWrite(OUT_IO, val?LOW:HIGH);
          }
          thExchange.WebInButtonState[i] = val > 0;
          
          if (thExchange.WebInButtonState[i])
          {
            thExchange.WebInButtonTime[i] = val + millis();
          }        
          break;
        }
      }
    }
    thExchange.Cnt_timeout = 0;      
    xSemaphoreGive(SemMutex);
*/

  }
  else
  {
    server.send(500, "text/plain", "invalid parameter(s)");
    return;
  }

  server.send(200,"text/plain", "ok");
}

void Web_execupdate()
{
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) 
  {
    Serial.printf("Update: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) 
    { //start with max available size
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) 
  {
    /* flashing firmware to ESP*/
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) 
    {
      Update.printError(Serial);
    }
  } 
  else if (upload.status == UPLOAD_FILE_END) 
  {
    if (Update.end(true)) 
    { //true to set the size to the current progress
      Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
    } 
    else 
    {
      Update.printError(Serial);
    }
  }
}

void TaskWeb( void * pvParameters )
{
  const char * pngs[] = { 
    "/robomower.png", 
    "/bat_empty.png" ,"/bat_low.png" ,"/bat_mid.png" ,"/bat_full.png",
    "/unlocked.png" ,"/locked.png" ,"/clock.png", 
    nullptr };
  int p = 0;
  
  if(!SPIFFS.begin(true))
    Serial.println(F("init SPIFFS error"));

  for (p=0; pngs[p]; p++)
  {
    server.on(pngs[p], [=]()
    {
      File dat = SPIFFS.open(pngs[p], "r");
      if (dat) 
      {
        server.send(200, "image/png", dat.readString());
        dat.close();
      }
    });
  }
    
  server.on("/", [=]()
  {
    File html = SPIFFS.open("/index.html", "r");
    if (html)
    {
      server.send(200, "text/html", html.readString());
      html.close();
    }
  });
  server.on("/update", HTTP_GET, [=]()
  {
    File html = SPIFFS.open("/update.html", "r");
    if (html)
    {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", html.readString());
      html.close();
    }
  });
  server.on("/execupdate", HTTP_POST, [=]() 
  {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, Web_execupdate );    
  
  server.on("/cmd", HTTP_GET, Web_getCmd);
  server.on("/web", Web_aktStatusWeb);
  server.on("/statval", Web_aktStatusValues);

  server.begin();
  //server.sendHeader("charset", "utf-8");

  while(1)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      server.handleClient();
    }
    delay(10);
  }
}
