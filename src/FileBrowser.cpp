
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <SPIFFS.h>


String formatBytes(size_t bytes);
String getContentType(String filename);
bool handleFileRead(String path);
void handleFileUpload();

extern WebServer server;
//holds the current upload
File fsUploadFile;

//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename){
  if (server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
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
/*        
        server.streamFile(file, contentType);
        file.close();
*/        
        return true;
    }
    return false;
}


void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    fsUploadFile = SPIFFS.open(filename, "w");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
  }
}

#if 0
void handleFileDelete() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (!exists(path)) {
    return server.send(404, "text/plain", "FileNotFound");
  }
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (exists(path)) {
    return server.send(500, "text/plain", "FILE EXISTS");
  }
  File file = SPIFFS.open(path, "w");
  if (file) {
    file.close();
  } else {
    return server.send(500, "text/plain", "CREATE FAILED");
  }
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  File root = SPIFFS.open("/");
  String output = "[";
  if(root.isDirectory()){
    File file = root.openNextFile();
    while(file) {
        if (output != "[") {
          output += ',';
        }
        output += "{";
        output += "\"name\":\"" + String(file.name()) + "\",";
        output += "\"size\":" + String(file.size());
        output += "}";

        file = root.openNextFile();
    }
  }
  output += "]";
  server.send(200, "text/json", output);
}
#endif