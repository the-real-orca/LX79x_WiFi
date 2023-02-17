#include <Arduino.h>
#include "SPIFFS.h"
#include "config_util.h"

bool readConfigFile(const char path[], void callback(String key, String value) )
{
    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.print("cannot open ini file: "); Serial.println(path);
        return false;
    } else {
        while ( file.available() ) {
            String line = file.readStringUntil('\n');
            int pos = line.indexOf('=');
            if ( pos > 0 ) {
                String key = line.substring(0, pos); key.trim();
                String value = line.substring(pos+1); value.trim();

                // execute callback
                callback(key, value);
            }
        }
    }
    file.close();

    return true;
}
