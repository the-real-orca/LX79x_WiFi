#include <stdint.h>

char DecodeChar (char raw);
int DecodeChars_IsRun (uint8_t raw[4]);
int DecodeChars_IsRunReady (uint8_t raw[4]);
char EncodeSeg (char c);
const char * DecodeMsg (char c1, char c2);
