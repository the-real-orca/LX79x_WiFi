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
char out[400];
LX790_State state;

// Request:   http://MOWERADRESS/web
// Response:  [cnt];[Display];[point];[lock];[clock];[bat];[rssi dbm];[Cnt_timeout];[Cnt_err];[LstError];[MowerStatustext]
void Web_aktStatusWeb()
{
  sprintf(out, "%0.1f;%c%c%c%c;%c;%d;%d;%d;%d;%d;%d;%d;%s", 
    millis()/1000.0,
    state.digits[0],state.digits[1],state.digits[2],state.digits[3],
    state.point,
    state.lock,
    state.clock,
    (state.mode==LX790_CHARGING) ? 4: state.battery,
    WiFi.RSSI(),
    0, // @todo Cnt_timeout, 
    0, // @todo Cnt_err, 
    0, // @todo Lst_err,
    state.msg);
    
  server.send(200,"text/plain", out);
}

// Request:   http://MOWERADRESS/statval
// Response:  [DisplayWithDelimiter];[rssi dbm];[batAsText];[MowerStatustext]
void Web_aktStatusValues()
{
  const char* BatState[] = {"off", "empty", "low", "mid", "full", "charging"};
  int IdxBatState = 0;
  
  if (state.mode == LX790_OFF)
    IdxBatState = 0;
  else if (state.mode == LX790_CHARGING)
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

String getContentType(String filename){
    if(filename.endsWith(".htm")) return "text/html";
    else if(filename.endsWith(".html")) return "text/html";
    else if(filename.endsWith(".css")) return "text/css";
    else if(filename.endsWith(".js")) return "application/javascript";
    else if(filename.endsWith(".png")) return "image/png";
    else if(filename.endsWith(".gif")) return "image/gif";
    else if(filename.endsWith(".jpg")) return "image/jpeg";
    else if(filename.endsWith(".ico")) return "image/x-icon";
    else if(filename.endsWith(".xml")) return "text/xml";
    else if(filename.endsWith(".json")) return "text/json";
    else if(filename.endsWith(".pdf")) return "application/x-pdf";
    else if(filename.endsWith(".zip")) return "application/x-zip";
    else if(filename.endsWith(".gz")) return "application/x-gzip";
    return "text/plain";
}

bool handleFileRead(String path) {
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    bool gZip = false;

    if ( path.indexOf("..") >= 0 ) {
        // security alert -> trying to exit web root
        return false;
    }

    if ( SPIFFS.exists(pathWithGz) )	{
        gZip = true;
        path = pathWithGz;
    }
    if ( gZip || SPIFFS.exists(path) ) {
        File file = SPIFFS.open(path, "r");

        if ( gZip && contentType != "application/x-gzip" && contentType != "application/octet-stream" ) {
            server.sendHeader("Content-Encoding", "gzip");
        }
        server.setContentLength(file.size());
        server.send(200, contentType, "");
        WiFiClient client = server.client();
        client.setNoDelay(true);
        client.write(file);
        file.close();
        return true;
    }
    return false;
}

void TaskWeb( void * pvParameters )
{
  memset(&state, 0x00, sizeof state);
  state.digits[0]='#'; state.digits[1]='#'; state.digits[2]='#'; state.digits[3]='#'; state.point=' ';
  state.msg = "";  

  if(!SPIFFS.begin(true))
    Serial.println(F("init SPIFFS error"));

  // redirect root to index.html
  server.on("/", HTTP_GET, [](){
      if(!handleFileRead("/index.html"))
          server.send(404, "text/plain", "FileNotFound");
  });

  // serve all files on SPIFFS
  server.onNotFound([](){
      if ( !handleFileRead(server.uri()) )
          server.send(404, "text/plain", "FileNotFound");
  });

  server.on("/execupdate", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, Web_execupdate );

  server.on("/cmd", HTTP_GET, Web_getCmd);
  server.on("/web", Web_aktStatusWeb);
  server.on("/statval", Web_aktStatusValues);

  server.begin();

  while(1)
  {
    // handle web server
    server.handleClient();

    // sync state
/*  
    {
      std::lock_guard<std::mutex> lock(stateMutex);
      if ( stateShared.updated )
        memcpy(&state, &stateShared, sizeof state);
    }
*/      
    

    delay(10);
  }
}
