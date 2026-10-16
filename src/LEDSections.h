#pragma once
#include "LEDController.h"
#include "FlashStorage.h"

// ============================================================
// HARDWARE: ONE 416-PIXEL DATA CHAIN ON ARDUINO PIN 6
// ============================================================
// Wiring order: Left 16-LED ring -> 48x8 matrix -> Right 16-LED ring.
// These are three *visual devices*, but one *electrical* NeoPixel strip.
// If they are connected to separate Arduino data pins, this must change.

#define STRIP_1_PIN 6
#define STRIP_1_COUNT 416   // 16 + (48 * 8) + 16
#define STRIP_2_PIN 7
#define STRIP_2_COUNT 0
#define STRIP_3_PIN 8
#define STRIP_3_COUNT 0
#define STRIP_4_PIN 9
#define STRIP_4_COUNT 0

Adafruit_NeoPixel strip1(STRIP_1_COUNT, STRIP_1_PIN, NEO_GRB + NEO_KHZ800);

#if STRIP_2_COUNT > 0
Adafruit_NeoPixel strip2(STRIP_2_COUNT, STRIP_2_PIN, NEO_GRB + NEO_KHZ800);
#endif
#if STRIP_3_COUNT > 0
Adafruit_NeoPixel strip3(STRIP_3_COUNT, STRIP_3_PIN, NEO_GRB + NEO_KHZ800);
#endif
#if STRIP_4_COUNT > 0
Adafruit_NeoPixel strip4(STRIP_4_COUNT, STRIP_4_PIN, NEO_GRB + NEO_KHZ800);
#endif

inline void registerStrips() {
  leds.attach(0, strip1);
#if STRIP_2_COUNT > 0
  leds.attach(1, strip2);
#endif
#if STRIP_3_COUNT > 0
  leds.attach(2, strip3);
#endif
#if STRIP_4_COUNT > 0
  leds.attach(3, strip4);
#endif
}

// ============================================================
// PHYSICAL LED RANGES (zero-based)
// ============================================================
// LeftRing    LEDs 0..15    (16)
// FacePanel   LEDs 16..399  (384 = 48 * 8)
// RightRing   LEDs 400..415 (16)
// Reversal / matrix zigzag is handled by ShapeDefinitions.h.

const LEDSpan allSpans[] SD_FLASH = {{0,   0, 416, false}};
const LEDSpan leftRingSpans[] SD_FLASH = {{0,   0,  16, false}};
const LEDSpan facePanelSpans[] SD_FLASH = {{0,  16, 384, false}};
const LEDSpan rightRingSpans[] SD_FLASH = {{0, 400,  16, false}};
const LEDSpan bothRingsSpans[] SD_FLASH = {
  {0,   0, 16, false},
  {0, 400, 16, false}
};

const LEDSection All SD_FLASH = {allSpans,       1, 416, {0, 0, 0, false}};
const LEDSection LeftRing SD_FLASH = {leftRingSpans,  1,  16, {0, 0, 0, false}};
const LEDSection FacePanel SD_FLASH = {facePanelSpans, 1, 384, {0, 0, 0, false}};
const LEDSection RightRing SD_FLASH = {rightRingSpans, 1,  16, {0, 0, 0, false}};
const LEDSection BothRings SD_FLASH = {bothRingsSpans, 2,  32, {0, 0, 0, false}};
