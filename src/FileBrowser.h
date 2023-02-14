#pragma once
#include <SPIFFS.h>

String formatBytes(size_t bytes);
String getContentType(String filename);
bool handleFileRead(String path);
void handleFileUpload();
void handleFileDelete();
bool exists(String path);