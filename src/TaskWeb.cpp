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
  sprintf(out, "{\"runtime\":%0.1f,"
                  "\"segments\":[%d,%d,%d,%d],"
                  "\"digits\":\"%c%c%c%c\","
                  "\"point\":\"%c\","
                  "\"lock\":%d, \"clock\":%d, \"wifi\":%d,"
                  "\"battery\":%d, \"brightness\":%d,"
                  "\"mode\":%d,"
                  "\"cmdQueue\":%d,"
                  "\"rssi\":%d,"
                  "\"msg\":\"%s\","
                  "\"build\":\"%s %s\"}",
    millis()/1000.0,
    state.segments[0],state.segments[1],state.segments[2],state.segments[3],
    state.digits[0],state.digits[1],state.digits[2],state.digits[3],
    state.point,
    state.lock, state.clock, state.wifi,
    (state.mode==LX790_CHARGING) ? 4: state.battery, state.brightness,
    state.mode,
    state.cmdQueueActive,
    WiFi.RSSI(),
    state.msg,
    __DATE__, __TIME__);
    
  server.send(200,"text/json", out);
}

//Webcommand examples: 
// Send command:         http://MOWERADRESS/cmd?parm=[command/button]&value=[state/time]
// Send command example: http://MOWERADRESS/cmd?parm=start&value=1
void Web_getCmd()
{
  if (server.argName(0) == "parm" &&
      server.argName(1) == "value")
  {
    String param = server.arg(0);
    int val = server.arg(1).toInt();
    CMD_Type cmd;

    if (server.arg(0) == "workzone" && val) {
      queueButton(BTN_OK, 3500);
    } else if (server.arg(0) == "timedate" && val) {
      queueButton(BTN_START, 3500);
    } else if (server.arg(0) == "startmow" && val) {
      queueButton(BTN_START, 250);
      cmd = {CMD_Type::WAIT, 250}; xQueueSend(cmdQueue, &cmd, 0);
      queueButton(BTN_OK, 250);
    } else if (server.arg(0) == "homemow" && val) {
      queueButton(BTN_HOME, 250);
      cmd = {CMD_Type::WAIT, 250}; xQueueSend(cmdQueue, &cmd, 0);
      queueButton(BTN_OK, 250);
    } else if (server.arg(0) == "setpin" && val) {
      cmd = {CMD_Type::BTN_PRESS, BTN_START}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::BTN_PRESS, BTN_HOME}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::WAIT, 5500}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::BTN_RELEASE, BTN_START}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::BTN_RELEASE, BTN_HOME}; xQueueSend(cmdQueue, &cmd, 0);
    } else if (server.arg(0) == "setstarttime" && val) {
      cmd = {CMD_Type::BTN_PRESS, BTN_START}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::BTN_PRESS, BTN_STOP}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::WAIT, 5500}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::BTN_RELEASE, BTN_START}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::BTN_RELEASE, BTN_STOP}; xQueueSend(cmdQueue, &cmd, 0);
    } else {
      for (int i=1; ButtonNames[i]; i++)
      {
        if (server.arg(0) == ButtonNames[i])
        {
          if ( val > 1 ) {
            queueButton(static_cast<BUTTONS>(i), val);
          } else if ( val ) {
            cmd = {CMD_Type::BTN_PRESS, i}; xQueueSend(cmdQueue, &cmd, 0);
          } else {
            cmd = {CMD_Type::BTN_RELEASE, i}; xQueueSend(cmdQueue, &cmd, 0);
          }
          break;
        }
      }
    state.cmdQueueActive = 1;
    }
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
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Update: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { 
      //start with max available size
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // flashing firmware to ESP
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("Update Success: %u Bytes\nRebooting...\n", upload.totalSize);
    } else {
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

  server.begin();

  while(1)
  {
    // handle web server
    server.handleClient();

    // sync state
    if ( xQueueReceive(stateQueue, &state, 0) == pdPASS ) {
    }


    delay(10);
  }
}
