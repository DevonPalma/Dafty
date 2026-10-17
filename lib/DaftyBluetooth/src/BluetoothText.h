#pragma once
// Guy Manuel BLE inbox. Only compiled in the BLE build environment.
#include <Arduino.h>
#include <ArduinoBLE.h>
#include <string.h>

namespace bluetoothText {
constexpr uint8_t MAX_CHARS = 63;
// Same UUIDs as the original marquee sketch (one service/one characteristic).
static BLEService service("19B10000-E8F2-537E-4F6C-D104768A1214");
static BLECharacteristic input("19B10001-E8F2-537E-4F6C-D104768A1214", BLEWrite, 64);
static char messageBuffer[MAX_CHARS + 1] = "";
static bool pending = false;

inline const char* message() { return messageBuffer; }
inline uint8_t length() { return uint8_t(strlen(messageBuffer)); }
inline bool begin() {
  if (!BLE.begin()) return false;
  BLE.setLocalName("Dafty_Marquee");
  BLE.setAdvertisedService(service);
  service.addCharacteristic(input);
  BLE.addService(service);
  BLE.advertise();
  return true;
}
inline void poll() {
  BLE.poll();
  if (!input.written()) return;
  int n = input.valueLength();
  if (n > MAX_CHARS) n = MAX_CHARS;
  if (n <= 0) return;
  const uint8_t* bytes = input.value();
  // A 5x7 font supports printable ASCII. Drop controls; stop at embedded NUL.
  uint8_t dst = 0;
  for (int i = 0; i < n && bytes[i]; ++i) {
    if (bytes[i] >= 32 && bytes[i] <= 126) messageBuffer[dst++] = char(bytes[i]);
  }
  messageBuffer[dst] = '\0';
  pending = dst != 0;
}
inline bool takeNewMessage() {
  const bool result = pending;
  pending = false;
  return result;
}
} // namespace bluetoothText
