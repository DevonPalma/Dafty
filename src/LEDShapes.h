#pragma once
// GEOMETRY VIEWS OVER EXISTING LED SECTIONS
// A shape does not allocate pixels. It maps local (x,y) or ring positions to
// indices within the referenced LEDSection, which LEDController maps to hardware.
// ShapeDefinitions.h configures actual named shapes and their orientation.


// LEDShapes.h - Geometry views over the LEDSection mapping.
// A shape NEVER owns pixels. It maps coordinates back to its section.
// Keep named shapes global/static: .on(shape) stores a pointer to them.
#include <Arduino.h>
#include <math.h>
#include "LEDSections.h"
#if defined(__AVR__)
#include <avr/pgmspace.h>
#endif

enum class ShapeKind : uint8_t { Line, Ring, Matrix, Custom };
enum class MatrixOrder : uint8_t { Rows, Columns };

struct LEDShape {
  const LEDSection* section;
  ShapeKind kind;
  uint16_t width;      // Matrix columns; ring circumference / line length
  uint16_t height;     // Matrix rows; 1 for a line/ring
  MatrixOrder order;
  bool zigzag;         // Reverse every other row/column
  bool reverse;        // Reverse ALL logical LED positions after mapping
  uint16_t offset;     // Rotation for rings (LED positions, not degrees)
  const uint16_t* lookup; // Optional logical XY/linear -> section index table
  bool lookupInFlash;  // Set true only for AVR PROGMEM lookup arrays
};

// Make shape descriptors. They use NO extra LED buffer.
constexpr LEDShape Line(const LEDSection& s, bool reverse = false) {
  return {&s, ShapeKind::Line, s.count, 1, MatrixOrder::Rows, false, reverse, 0, nullptr, false};
}
constexpr LEDShape Ring(const LEDSection& s, uint16_t startOffset = 0, bool clockwise = true) {
  return {&s, ShapeKind::Ring, s.count, 1, MatrixOrder::Rows, false, !clockwise, startOffset, nullptr, false};
}
constexpr LEDShape Matrix(const LEDSection& s, uint16_t w, uint16_t h,
                          MatrixOrder order = MatrixOrder::Rows,
                          bool zigzag = true, bool reverse = false) {
  return {&s, ShapeKind::Matrix, w, h, order, zigzag, reverse, 0, nullptr, false};
}
// Custom mappings are local LED indices, NOT physical strip indices.
// The table has `count` entries and should have static lifetime.
constexpr LEDShape CustomShape(const LEDSection& s, const uint16_t* map,
                                uint16_t w, uint16_t h = 1,
                                bool flash = false) {
  return {&s, ShapeKind::Custom, w, h, MatrixOrder::Rows, false, false, 0, map, flash};
}

namespace shape {
  // INVALID is returned rather than writing to an unrelated LED.
  static const uint16_t INVALID = 0xFFFF;

  inline uint16_t mappedIndex(const LEDShape& sh, uint16_t x, uint16_t y = 0) {
    if (!sh.section || !sh.width || !sh.height || x >= sh.width || y >= sh.height) return INVALID;
    uint32_t total = (uint32_t)sh.width * sh.height;
    if (total > 65535UL) return INVALID;
    uint32_t logical = (uint32_t)y * sh.width + x;
    uint32_t index = logical;
    if (sh.kind == ShapeKind::Matrix) {
      if (sh.order == MatrixOrder::Rows)
        index = (uint32_t)y * sh.width + (sh.zigzag && (y & 1) ? sh.width - 1 - x : x);
      else
        index = (uint32_t)x * sh.height + (sh.zigzag && (x & 1) ? sh.height - 1 - y : y);
    } else if (sh.kind == ShapeKind::Ring) {
      index = (x + (uint32_t)(sh.offset % sh.width)) % sh.width;
    }
    if (sh.kind == ShapeKind::Custom) {
      if (!sh.lookup) return INVALID;
#if defined(__AVR__)
      index = sh.lookupInFlash ? pgm_read_word(&sh.lookup[logical]) : sh.lookup[logical];
#else
      index = sh.lookup[logical];
#endif
    } else if (sh.reverse) {
      index = total - 1 - index;
    }
    return index < sh.section->count ? (uint16_t)index : INVALID;
  }

  inline void setXY(const LEDShape& sh, uint16_t x, uint16_t y, Color c) {
    uint16_t i = mappedIndex(sh,x,y);
    if (i != INVALID) leds.set(*sh.section, i, c);
  }
  inline void set(const LEDShape& sh, uint16_t index, Color c) {
    if (!sh.width) return;
    setXY(sh, index % sh.width, index / sh.width, c);
  }
  inline void fill(const LEDShape& sh, Color c) {
    if (!sh.section) return;
    for (uint16_t y=0; y<sh.height; ++y)
      for (uint16_t x=0; x<sh.width; ++x) setXY(sh,x,y,c);
  }
  // Angle 0 = first LED. Angles increase clockwise unless ring is reversed.
  inline void setAngle(const LEDShape& ring, float degrees, Color c) {
    if (ring.kind != ShapeKind::Ring || !ring.width) return;
    float wrapped = fmodf(degrees,360.0f);
    if (wrapped < 0) wrapped += 360.0f;
    uint16_t i = (uint16_t)(wrapped * ring.width / 360.0f);
    set(ring,i,c);
  }
}
