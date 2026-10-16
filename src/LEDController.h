// PHYSICAL LED ACCESS LAYER
// Converts named logical sections into pixels on up to four NeoPixel strips.
// LEDSections.h supplies the wiring, strip registration, and section spans.
// Effects should use leds.set(section, index, color) or leds.fill(section,
// color), not raw strip offsets, so wiring changes stay in LEDSections.h.

#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "FlashStorage.h"

// One controller handles up to four independently wired NeoPixel strips.
// All indices in LEDSection are LOCAL to their named section.
// A span describes one continuous physical range on a registered strip.
// Its strip index is zero-based even though wiring labels are STRIP_1, etc.
struct LEDSpan {
  uint8_t strip;        // 0, 1, 2, 3
  uint16_t start;       // First physical LED on that strip
  uint16_t count;
  bool reversed;        // Map local index backwards within this span
};

// A logical section concatenates one or more spans. `count` must equal
// the sum of their lengths; set() addresses indices from 0 to count - 1.
// Multiple named sections may intentionally reference the same pixels.
struct LEDSection {
  const LEDSpan* spans;
  uint8_t spanCount;
  uint16_t count;
  // Cached first span, populated in resolveTarget() once per effect.
  // Only used for single-span sections. Never dereference PROGMEM per pixel.
  LEDSpan fastSpan;
};

// Colors are 8-bit RGB values. RGB() and the constants below are helpers
// for effects and playlists; setBrightness(100) applies per physical strip.
struct Color {
  uint8_t r, g, b;
};

constexpr Color Red    = {255, 0, 0};
constexpr Color Green  = {0, 255, 0};
constexpr Color Blue   = {0, 0, 255};
constexpr Color Purple = {160, 0, 255};
constexpr Color Yellow = {255, 200, 0};
constexpr Color Cyan   = {0, 255, 255};
constexpr Color White  = {255, 255, 255};
constexpr Color Black  = {0, 0, 0};
inline Color RGB(uint8_t r, uint8_t g, uint8_t b) {
  return {r, g, b};
}

// Attach registered physical strips before begin(); clear() modifies their
// buffers only, while show() transmits those buffers to the actual LEDs.
class LEDController {
    Adafruit_NeoPixel* strips[4] = {nullptr, nullptr, nullptr, nullptr};
  public:
    bool attach(uint8_t index, Adafruit_NeoPixel& strip) {
      if (index >= 4) return false;
      strips[index] = &strip;
      return true;
    }
    void begin() {
      for (uint8_t i = 0; i < 4; ++i) if (strips[i]) {
          strips[i]->begin();
          strips[i]->setBrightness(100);
          strips[i]->clear();
          strips[i]->show();
        }
    }
    void clear() {
      for (uint8_t i = 0; i < 4; ++i) if (strips[i]) strips[i]->clear();
    }
    void show() {
      for (uint8_t i = 0; i < 4; ++i) if (strips[i]) strips[i]->show();
    }
    uint16_t count(uint8_t index) const {
      return index < 4 && strips[index] ? strips[index]->numPixels() : 0;
    }
    // Map a logical section index through its spans to one physical LED.
    // Reversed spans flip ordering within that span, not the entire section.
    // Out-of-range LEDs or missing strips are ignored safely.
    void set(const LEDSection& section, uint16_t local, Color color) {
      if (local >= section.count) return;
      if (section.spanCount == 1) {
        const LEDSpan& sp = section.fastSpan;
        if (sp.strip >= 4 || !strips[sp.strip] || local >= sp.count) return;
        uint16_t physical = sp.start + (sp.reversed ? sp.count - 1 - local : local);
        if (physical < strips[sp.strip]->numPixels())
          strips[sp.strip]->setPixelColor(physical, color.r, color.g, color.b);
        return;
      }
      for (uint8_t j = 0; j < section.spanCount; ++j) {
        LEDSpan s = flashRead(section.spans + j);
        if (local < s.count) {
          if (s.strip >= 4 || !strips[s.strip]) return;
          uint16_t physical = s.start + (s.reversed ? s.count - 1 - local : local);
          if (physical >= strips[s.strip]->numPixels()) return;
          strips[s.strip]->setPixelColor(physical, color.r, color.g, color.b);
          return;
        }
        local -= s.count;
      }
    }
    void fill(const LEDSection& section, Color color) {
      if (section.spanCount == 1) {
        const LEDSpan& sp = section.fastSpan;
        if (sp.strip >= 4 || !strips[sp.strip]) return;
        Adafruit_NeoPixel& strip = *strips[sp.strip];
        for (uint16_t i = 0; i < sp.count; ++i) {
          uint16_t index = sp.start + (sp.reversed ? sp.count - 1 - i : i);
          if (index < strip.numPixels()) strip.setPixelColor(index, color.r, color.g, color.b);
        }
        return;
      }
      for (uint16_t i = 0; i < section.count; ++i) set(section, i, color);
    }
};
extern LEDController leds;
