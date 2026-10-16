#pragma once
// Compile-time LED geometry. No sections, shape descriptors or lookup arrays.
// Every index maps directly to the ONE physical NeoPixel chain.
// Change these aliases to match your hardware, then rebuild.
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

constexpr uint16_t LED_COUNT = 416;
constexpr uint8_t LED_PIN = 6;
constexpr uint16_t INVALID_LED = 0xFFFF;
extern Adafruit_NeoPixel pixels; // Defined once in main.cpp.

struct Color
{
  uint8_t r, g, b;
};
constexpr Color Red = {255, 0, 0}, Green = {0, 255, 0}, Blue = {0, 0, 255};
constexpr Color Purple = {160, 0, 255}, Yellow = {255, 200, 0}, Cyan = {0, 255, 255};
constexpr Color White = {255, 255, 255}, Black = {0, 0, 0};
inline Color RGB(uint8_t r, uint8_t g, uint8_t b) { return {r, g, b}; }

inline void writePhysical(uint16_t i, Color c)
{
  if (i < LED_COUNT)
    pixels.setPixelColor(i, c.r, c.g, c.b);
}

enum class MatrixOrder : uint8_t
{
  Rows,
  Columns
};

template <uint16_t START, uint16_t COUNT, bool REVERSE = false>
struct Strip
{
  static constexpr uint16_t start = START, count = COUNT, width = COUNT, height = 1;
  static constexpr bool isMatrix = false;
  static inline uint16_t index(uint16_t i)
  {
    if (i >= COUNT)
      return INVALID_LED;
    return START + (REVERSE ? COUNT - 1 - i : i);
  }
  static inline uint16_t indexXY(uint16_t x, uint16_t y) { return y == 0 ? index(x) : INVALID_LED; }
  static inline void set(uint16_t i, Color c) { writePhysical(index(i), c); }
  static inline void setXY(uint16_t x, uint16_t y, Color c) { writePhysical(indexXY(x, y), c); }
};

template <uint16_t START, uint16_t COUNT, uint16_t OFFSET = 0, bool CLOCKWISE = true>
struct Ring
{
  static constexpr uint16_t start = START, count = COUNT, width = COUNT, height = 1;
  static constexpr bool isMatrix = false;
  static inline uint16_t index(uint16_t i)
  {
    if (i >= COUNT)
      return INVALID_LED;
    const uint16_t shifted = (i + OFFSET % COUNT) % COUNT;
    return START + (CLOCKWISE ? shifted : COUNT - 1 - shifted);
  }
  static inline uint16_t indexXY(uint16_t x, uint16_t y) { return y == 0 ? index(x) : INVALID_LED; }
  static inline void set(uint16_t i, Color c) { writePhysical(index(i), c); }
  static inline void setXY(uint16_t x, uint16_t y, Color c) { writePhysical(indexXY(x, y), c); }
};

template <uint16_t START, uint16_t W, uint16_t H, MatrixOrder ORDER,
          bool SERPENTINE = false, bool REVERSE = false>
struct Matrix
{
  static_assert(W > 0 && H > 0, "Matrix dimensions must be nonzero");
  static_assert((uint32_t)W * H <= 65535UL, "Matrix too large");
  static constexpr uint16_t start = START, width = W, height = H, count = W * H;
  static constexpr bool isMatrix = true;
  static inline uint16_t indexXY(uint16_t x, uint16_t y)
  {
    if (x >= W || y >= H)
      return INVALID_LED;
    uint16_t i;
    if (ORDER == MatrixOrder::Rows)
      i = y * W + (SERPENTINE && (y & 1) ? W - 1 - x : x);
    else
      i = x * H + (SERPENTINE && (x & 1) ? H - 1 - y : y);
    if (REVERSE)
      i = W * H - 1 - i;
    return START + i;
  }
  // Logical row-major index, not physical wiring order.
  static inline uint16_t index(uint16_t i)
  {
    return i < count ? indexXY(i % W, i / W) : INVALID_LED;
  }
  static inline void set(uint16_t i, Color c) { writePhysical(index(i), c); }
  static inline void setXY(uint16_t x, uint16_t y, Color c) { writePhysical(indexXY(x, y), c); }
};

template <typename A, typename B>
struct Group
{
  static constexpr uint16_t start = 0, count = A::count + B::count, width = count, height = 1;
  static constexpr bool isMatrix = false;
  static inline uint16_t index(uint16_t i)
  {
    if (i < A::count)
      return A::index(i);
    return i < count ? B::index(i - A::count) : INVALID_LED;
  }
  static inline uint16_t indexXY(uint16_t x, uint16_t y) { return y == 0 ? index(x) : INVALID_LED; }
  static inline void set(uint16_t i, Color c) { writePhysical(index(i), c); }
  static inline void setXY(uint16_t x, uint16_t y, Color c) { writePhysical(indexXY(x, y), c); }
};

// Single electrical chain: ring 0..15, face 16..399, ring 400..415.
using All = Strip<0, 416>;
using LeftRing = Ring<0, 16>;
using FacePanel = Strip<16, 384>;
using FaceMatrix = Matrix<16, 48, 8, MatrixOrder::Rows, false>; // progressive Wokwi
using RightRing = Ring<400, 16>;
using BothRings = Group<LeftRing, RightRing>;
// When switching Wokwi matrix to serpentine, change ONLY the last false above.

// Fast section clearing uses the NeoPixel buffer directly (doesn't transmit).
inline void clearPixels() { pixels.clear(); }
inline void showPixels() { pixels.show(); }
