#pragma once
// Common color and timing helpers. No buffers and no floating-point math.
// All animations operate directly on the ONE NeoPixel framebuffer.
#include "../LEDLayouts.h"

namespace anim
{

  inline uint8_t scale8(uint8_t channel, uint8_t amount)
  {
    return uint8_t((uint16_t(channel) * amount + 127u) / 255u);
  }

  inline Color dim(Color color, uint8_t brightness)
  {
    return {scale8(color.r, brightness), scale8(color.g, brightness),
            scale8(color.b, brightness)};
  }

  inline Color blend(Color a, Color b, uint8_t fraction)
  {
    const uint8_t inverse = 255 - fraction;
    return {uint8_t((uint16_t(a.r) * inverse + uint16_t(b.r) * fraction) / 255),
            uint8_t((uint16_t(a.g) * inverse + uint16_t(b.g) * fraction) / 255),
            uint8_t((uint16_t(a.b) * inverse + uint16_t(b.b) * fraction) / 255)};
  }

  // Returns a complete up/down triangle cycle from 0 to 255 to 0.
  inline uint8_t triangle(uint32_t elapsedMs, uint16_t periodMs)
  {
    if (periodMs < 2)
      return 0;
    const uint16_t phase = elapsedMs % periodMs;
    const uint16_t half = periodMs / 2;
    const uint16_t rise = phase <= half ? phase : periodMs - phase;
    return uint8_t((uint32_t(rise) * 255u) / half);
  }

  // Repeatable noise; no random() state or per-animation storage.
  inline uint16_t hash16(uint16_t x)
  {
    x ^= x >> 7;
    x *= 40503u;
    x ^= x >> 5;
    return x;
  }

  // Compact six-section color wheel. Hue wraps naturally at 255.
  inline Color wheel(uint8_t hue)
  {
    const uint8_t sector = hue / 43;
    const uint8_t offset = uint8_t((uint16_t(hue % 43) * 255) / 43);
    switch (sector)
    {
    case 0:
      return {255, offset, 0};
    case 1:
      return {uint8_t(255 - offset), 255, 0};
    case 2:
      return {0, 255, offset};
    case 3:
      return {0, uint8_t(255 - offset), 255};
    case 4:
      return {offset, 0, 255};
    default:
      return {255, 0, uint8_t(255 - offset)};
    }
  }

  // Animation clock in 0..255. One complete cycle lasts periodMs.
  inline uint8_t phase8(uint32_t elapsedMs, uint16_t periodMs)
  {
    return periodMs ? uint8_t(((elapsedMs % periodMs) * 256UL) / periodMs) : 0;
  }

} // namespace anim

// Compatibility with the previous 3-animation version.
inline Color dimColor(Color color, uint8_t level) { return anim::dim(color, level); }
