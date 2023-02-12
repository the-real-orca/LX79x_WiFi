#include <Arduino.h>
#include <string.h>
#include <Wire.h>
//#include "CRC16.h"
#include "CRC.h"
#include <WiFi.h>
#include <WebServer.h>
#include "SPIFFS.h"
#include <Update.h>
#define EZTIME_EZT_NAMESPACE
#include <ezTime.h>
#include "LX790_util.h"

static const char* StatusFiles[] = {"/status.log", "/status1.log", "/status2.log"};
static const uint8_t StatusFilesCount = 3;
static const uint16_t StatusLinesCount = 500;

// Copy "config_sample.h" to "config.h" and change it according to your setup.
#include "config.h"

WebServer server(80);
static char out[512];
LX790_State state;

const char *UPDATE_HTML =
  #include "update.html.h"
;

boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

// checks if the request is for the controllers IP, if not we redirect automatically to the captive portal 
boolean captivePortal() {
  if (!isIp(server.hostHeader())) {
    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()) + String("/update"), true);
    server.send(302, "text/plain", "");   
    server.client().stop(); 
    return true;
  }
  return false;
}


String formatBytes(size_t bytes);
String getContentType(String filename);
bool exists(String path);
bool handleFileRead(String path);
void handleFileUpload();
void handleFileDelete();

const char *jsonStatus() {

  //  handle NTP time
  String utcStr="---";
  if ( ezt::timeStatus() == timeStatus_t::timeSet )
    utcStr = UTC.dateTime("Y-m-d H:i:s T");

  sprintf(out, "{\"time\":\"%s\", "
                  "\"runtime\":%0.1f, "
                  "\"segments\":[%d,%d,%d,%d], "
                  "\"digits\":\"%c%c%c%c\", "
                  "\"point\":\"%c\", "
                  "\"lock\":%d, \"clock\":%d, "
                  "\"battery\":%d, \"brightness\":%d, "
                  "\"mode\":\"%s\", "
                  "\"cmdQueue\":%d, "
                  "\"wifi\":%d, \"rssi\":%d, \"hostname\":\"%s\", "
                  "\"msg\":\"%s\", "
                  "\"autoUnlock\":%d, "
                  "\"debugLog\":%d, "
                  "\"build\":\"%s %s\"}",
    utcStr.c_str(),
    millis()/1000.0,
    state.segments[0],state.segments[1],state.segments[2],state.segments[3],
    state.digits[0],state.digits[1],state.digits[2],state.digits[3],
    state.point,
    state.lock, state.clock, 
    state.battery, state.brightness,
    ModeNames[state.mode],
    uxQueueMessagesWaiting(cmdQueue),
    state.wifi, WiFi.RSSI(),
    state.hostname,
    state.msg,
    state.autoUnlock, state.debugLog,
    __DATE__, __TIME__);

  return out;
}

void Web_getLog()
{
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/json", "");
  server.sendContent("[");

  for (int i = StatusFilesCount-1; i >= 0; i--) {
    File file = SPIFFS.open(StatusFiles[i], "r");
    if ( file ) {
      String line = file.readStringUntil('\n');
      while ( !line.isEmpty() ) {
        server.sendContent(line);
        line = file.readStringUntil('\n');
      }
      file.close();
    }
  }
  server.sendContent("{}]");
  server.sendContent("");
}

// Request:   http://MOWERADRESS/web
// Response:  [cnt];[Display];[point];[lock];[clock];[bat];[rssi dbm];[Cnt_timeout];[Cnt_err];[LstError];[MowerStatustext]
void Web_aktStatusWeb()
{
  server.send(200,"text/json", jsonStatus());
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

    if (param == "reboot" && val) {
      cmd = {CMD_Type::REBOOT, 200}; xQueueSend(cmdQueue, &cmd, 0);
    } else if (param == "startmow" && val) {
      queueButton(BTN_START, 250);
      cmd = {CMD_Type::WAIT, 250}; xQueueSend(cmdQueue, &cmd, 0);
      queueButton(BTN_OK, 250);
    } else if (param == "homemow" && val) {
      queueButton(BTN_HOME, 250);
      cmd = {CMD_Type::WAIT, 250}; xQueueSend(cmdQueue, &cmd, 0);
      queueButton(BTN_OK, 250);
    } else if (param == "workzone" && val) {
      queueButton(BTN_OK, 4500);
    } else if (param == "timedate" && val) {
      queueButton(BTN_START, 4500);
    } else if (param == "setpin" && val) {
      cmd = {CMD_Type::BTN_PRESS, BTN_START}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::BTN_PRESS, BTN_HOME}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::WAIT, 8000}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::BTN_RELEASE, BTN_START}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::BTN_RELEASE, BTN_HOME}; xQueueSend(cmdQueue, &cmd, 0);
    } else if (param == "setstarttime" && val) {
      cmd = {CMD_Type::BTN_PRESS, BTN_START}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::BTN_PRESS, BTN_STOP}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::WAIT, 8000}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::BTN_RELEASE, BTN_START}; xQueueSend(cmdQueue, &cmd, 0);
      cmd = {CMD_Type::BTN_RELEASE, BTN_STOP}; xQueueSend(cmdQueue, &cmd, 0);
    } else if (param == "debugLog") {
      cmd = {CMD_Type::DEBUGLOG, val}; xQueueSend(cmdQueue, &cmd, 0);
    } else if (param == "autoUnlock") {
      cmd = {CMD_Type::AUTOUNLOCK, val}; xQueueSend(cmdQueue, &cmd, 0);
    } else {
      // @todo prioritize emergency stop !!!

      for (int i=1; ButtonNames[i]; i++)
      {
        if (param == ButtonNames[i])
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
    #if DEBUG_SERIAL_PRINT
      Serial.printf("Update: %s\n", upload.filename.c_str());
    #endif
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

void TaskWeb( void * pvParameters )
{
  memset(&state, 0x00, sizeof state);
  state.digits[0]='#'; state.digits[1]='#'; state.digits[2]='#'; state.digits[3]='#'; state.point=' ';
  state.msg="";  

  if(!SPIFFS.begin(true))
    Serial.println(F("init SPIFFS error"));

  // redirect root to index.html
  server.on("/", HTTP_GET, [](){
    if(!handleFileRead("/index.html"))
        server.send(404, "text/plain", "FileNotFound");
  });

  // serve all files on SPIFFS
  server.onNotFound([](){
//    Serial.printf("request URL: %s\n", server.uri());
    if ( captivePortal() )
      return;

    if ( !handleFileRead(server.uri()) )
        server.send(404, "text/plain", "FileNotFound");
  });

  server.on("/update", HTTP_GET, []() {
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", UPDATE_HTML);

    server.sendContent("<ul>");

    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while(file) {
      sprintf(out, "<li><a href='%s'>%s</a> (%s)  <b><a href='del?file=%s'>X</a></b></li>",
        file.name(), file.name(),
        formatBytes(file.size()).c_str(),
        file.name()
      );
      server.sendContent(out);
      file = root.openNextFile();
    }
    server.sendContent("</ul>");
    server.sendContent("</body></html>");
    server.sendContent("");

     
  });
  server.on("/fileupload", HTTP_POST, []() {
    server.sendHeader("Location", "/update");
    server.send(301, "text/plain", "OK");
  }, handleFileUpload);

  server.on("/execupdate", HTTP_POST, []() {
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    CMD_Type cmd = {CMD_Type::REBOOT, 500}; xQueueSend(cmdQueue, &cmd, 0);
  }, Web_execupdate );


  server.on("/cmd", HTTP_GET, Web_getCmd);
  server.on("/web", HTTP_GET, Web_aktStatusWeb);
  server.on("/log", HTTP_GET, Web_getLog);
  server.on("/del", HTTP_GET, handleFileDelete);

  server.enableCORS(true);
  server.begin();

  while(1)
  {
    // handle web server
    server.handleClient();

    // sync state
    if ( xQueueReceive(stateQueue, &state, 0) == pdPASS ) {
      
      // save to log
      if ( state.debugLog ) {
        File file = SPIFFS.open(StatusFiles[0], "a");
        if ( file ) {
          file.print( jsonStatus() ); file.println(",");
          size_t filesize = file.size();
          file.close();

          // rotate log file
          if ( filesize > (250*StatusLinesCount) ) {
            for (int i = (StatusFilesCount-1); i > 0; i--) {
              SPIFFS.remove(StatusFiles[i]);
              SPIFFS.rename(StatusFiles[i-1], StatusFiles[i]);
            }
          }
        }
      }
    }

    // handle NTP
    ezt::events();

    delay(10);
  }

}
