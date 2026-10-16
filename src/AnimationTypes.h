#pragma once
#include <Arduino.h>
#include <string.h>
#include "LEDShapes.h"

// INTERNAL RENDER ADAPTER, NOT A PLAYLIST FORMAT
// The Flash scheduler builds ONE temporary Effect on the stack while
// rendering a FlashEffect. Permanent definitions live in PROGMEM.
// Legacy fluent builders have been removed to prevent accidentally bringing
// RAM-backed effects back into the program.
//
// The maximum settings size among the existing effects is below 24 bytes.
#define EFFECT_CONFIG_BYTES 24

struct Effect;
typedef void (*EffectRender)(const Effect&, const LEDSection&, float);

struct Effect {
  const LEDShape* shape;      // Optional STACK-owned geometry view.
  const LEDSection* section;  // STACK-owned section snapshot.
  EffectRender render;        // Renderer function (code lives in Flash).
  uint8_t config[EFFECT_CONFIG_BYTES]; // One effect's settings, temporarily.
};

// Settings structs must be trivially copyable and <= EFFECT_CONFIG_BYTES.
template<typename T>
inline T effectSettings(const Effect& effect) {
  static_assert(sizeof(T) <= EFFECT_CONFIG_BYTES, "Effect settings too large");
  T settings;
  memcpy(&settings, effect.config, sizeof(T));
  return settings;
}
