#pragma once
// CROSS-PLATFORM READ-ONLY STORAGE
// On AVR (Uno R3), SD_FLASH puts static tables into Flash (PROGMEM),
// and flashRead() copies just one record to the stack with memcpy_P.
// On other architectures, normal const-data reads are sufficient.
// Never use a PROGMEM pointer as a normal RAM pointer on AVR.


#include <Arduino.h>
#include <string.h>
#if defined(__AVR__)
#include <avr/pgmspace.h>
#define SD_FLASH PROGMEM
#else
#define SD_FLASH
#endif

template<typename T>
inline T flashRead(const T* pointer) {
  T copy;
#if defined(__AVR__)
  memcpy_P(&copy, pointer, sizeof(T));
#else
  memcpy(&copy, pointer, sizeof(T));
#endif
  return copy;
}
