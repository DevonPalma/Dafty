// OPTIONAL SERIAL DIAGNOSTICS
// Set DEBUG_ENABLED to false to compile out DEBUG_PRINT/DEBUG_PRINTLN and
// DEBUG_START calls. When enabled, logs use 115200 baud; open Serial Monitor
// at that speed. The F() wrapper in call sites keeps string literals in flash
// on supported Arduino boards, reducing SRAM consumption.

#pragma once
#include <Arduino.h>

// Enable or disable serial debugging
#define DEBUG_ENABLED false

#if DEBUG_ENABLED

#define DEBUG_START() Serial.begin(115200)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)

#else

#define DEBUG_START() ((void)0)
#define DEBUG_PRINT(x) ((void)0)
#define DEBUG_PRINTLN(x) ((void)0)

#endif