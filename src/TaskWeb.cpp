#include <Arduino.h>
#include <string.h>
#include <Wire.h>
//#include "CRC16.h"
#include "CRC.h"
#include <WiFi.h>
#include <WebServer.h>
#include "SPIFFS.h"
#include "FileBrowser.h"
#include <Update.h>
#define EZTIME_EZT_NAMESPACE
#include <ezTime.h>
#include "LX790_util.h"

static const char* DebugFiles[] = {"/debug.log", "/debug1.log", "/debug2.log", "/debug3.log", "/debug4.log"};
static const uint8_t DebugFilesCount = 5;
static const uint16_t DebugFileMaxSize = 65530;

static const char* LogFiles[] = {"/system.log", "/system1.log", "/system2.log", "/system3.log", "/system4.log"};
static const uint8_t LogFilesCount = 5;
static const uint16_t LogFileMaxSize = 65530;

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
    DEBUG_println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()) + String("/config"), true);
    server.send(302, "text/plain", "");   
    server.client().stop(); 
    return true;
  }
  return false;
}



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
    state.hostname.c_str(),
    state.msg.c_str(),
    state.autoUnlock, state.debugLog,
    __DATE__, __TIME__);

  return out;
}

void Web_getLog()
{
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/json", "");
  server.sendContent("[");

  for (int i = LogFilesCount-1; i >= 0; i--) {
    File file = SPIFFS.open(LogFiles[i], "r");
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

void Web_getDebugLog()
{
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/json", "");
  server.sendContent("[");

  for (int i = DebugFilesCount-1; i >= 0; i--) {
    File file = SPIFFS.open(DebugFiles[i], "r");
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
    DEBUG_printf("Update: %s\n", upload.filename.c_str());
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

  // wait for HW task to start up network
  delay(2000);

  // redirect root to index.html
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/config", SPIFFS, "/config.html");

  // serve all files on SPIFFS
  server.onNotFound([](){
    DEBUG_printf("request URL: %s\n", server.uri());
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

    size_t total = SPIFFS.totalBytes();
    size_t used = SPIFFS.usedBytes();    
    server.sendContent("<p>");
    server.sendContent("Size Total: "); server.sendContent(formatBytes(total).c_str());
    server.sendContent(" / Used: "); server.sendContent(formatBytes(used).c_str());
    server.sendContent(" / Free: "); server.sendContent(formatBytes(total-used).c_str());
    server.sendContent("</p>");

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
  server.on("/status", HTTP_GET, Web_aktStatusWeb);
  server.on("/debuglog", HTTP_GET, Web_getDebugLog);
  server.on("/log", HTTP_GET, Web_getLog);
  server.on("/del", HTTP_GET, handleFileDelete);

  server.on("/config.json", HTTP_PUT, []() {
    if (server.args() == 0) {
      return server.send(500, "text/plain", "BAD ARGS");
    }
    String data = server.arg(0);
    if ( data.length() < 2 )
      return server.send(500, "text/plain", "BAD CONFIG");

    File file = SPIFFS.open("/config.json", "w");
    file.println(data);
    file.close();
    server.send(200,"text/plain", "ok");

    DEBUG_println("config.json saved");
  });

  server.enableCORS(true);
  server.begin();

  while(1)
  {
    // handle web server
    server.handleClient();

    // sync state
    if ( xQueueReceive(stateQueue, &state, 0) == pdPASS ) {
      
      // save log on status change
      static LX790_Mode lastMode = LX790_UNKNOWN;

      if ( state.mode != lastMode ) {
        lastMode = state.mode;
        File file = SPIFFS.open(LogFiles[0], "a");
        if ( file ) {
          file.print( jsonStatus() ); file.println(",");
          size_t filesize = file.size();
          file.close();

          // rotate log file
          if ( filesize > LogFileMaxSize ) {
            for (int i = (LogFilesCount-1); i > 0; i--) {
              SPIFFS.remove(LogFiles[i]);
              SPIFFS.rename(LogFiles[i-1], LogFiles[i]);
            }
          }
        }
      }

      // save to debug log
      if ( state.debugLog ) {
        File file = SPIFFS.open(DebugFiles[0], "a");
        if ( file ) {
          file.print( jsonStatus() ); file.println(",");
          size_t filesize = file.size();
          file.close();

          // rotate log file
          if ( filesize > DebugFileMaxSize ) {
            for (int i = (DebugFilesCount-1); i > 0; i--) {
              SPIFFS.remove(DebugFiles[i]);
              SPIFFS.rename(DebugFiles[i-1], DebugFiles[i]);
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
