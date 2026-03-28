#pragma once
#include <SPIFFS.h>

/**
 * @brief Formats a byte size into a human-readable string (B, KB, MB, GB).
 * @param bytes The size in bytes.
 * @return A formatted string.
 */
String formatBytes(size_t bytes);

/**
 * @brief Determines the MIME content type based on the file extension.
 * @param filename The name of the file.
 * @return The corresponding MIME type string.
 */
String getContentType(String filename);

/**
 * @brief Handles reading a file from the SPIFFS and sending it to the client.
 * Supports Gzip compression if a .gz version of the file exists.
 * @param path The path to the file.
 * @return true if the file was found and sent, false otherwise.
 */
bool handleFileRead(String path);

/**
 * @brief Handles file uploads from the web server.
 * Manages the different states of the upload (START, WRITE, END).
 */
void handleFileUpload();

/**
 * @brief Handles file deletion requests from the web server.
 * Expects the path to be provided as a server argument.
 */
void handleFileDelete();

/**
 * @brief Checks if a file exists in the SPIFFS.
 * @param path The path to the file.
 * @return true if the file exists, false otherwise.
 * @note This function declaration is present but implementation might be missing or handled by SPIFFS.exists.
 */
bool exists(String path);