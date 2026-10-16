#pragma once
// Tiny 2D drawing utilities. Zero heap allocations or additional LED buffers.
// Pass an LEDShape already resolved from PROGMEM by renderFlashEffect().
// Coordinates start at the TOP LEFT, with Y increasing downward.
#include "LEDShapes.h"

class MatrixGraphics {
  const LEDShape& m;
public:
  explicit MatrixGraphics(const LEDShape& matrix) : m(matrix) {}
  uint16_t width() const { return m.width; }
  uint16_t height() const { return m.height; }
  void pixel(int16_t x, int16_t y, Color c) const {
    if (x < 0 || y < 0 || x >= (int16_t)m.width || y >= (int16_t)m.height) return;
    shape::setXY(m, (uint16_t)x, (uint16_t)y, c);
  }
  void hline(int16_t x, int16_t y, int16_t length, Color c) const {
    if (y < 0 || y >= (int16_t)m.height || length <= 0) return;
    int16_t end = x + length;
    if (x < 0) x = 0;
    if (end > (int16_t)m.width) end = m.width;
    for (; x < end; ++x) shape::setXY(m, x, y, c);
  }
  void vline(int16_t x, int16_t y, int16_t length, Color c) const {
    if (x < 0 || x >= (int16_t)m.width || length <= 0) return;
    int16_t end = y + length;
    if (y < 0) y = 0;
    if (end > (int16_t)m.height) end = m.height;
    for (; y < end; ++y) shape::setXY(m, x, y, c);
  }
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, Color c) const {
    if (w <= 0 || h <= 0) return;
    int16_t endY = y + h;
    if (y < 0) y = 0;
    if (endY > (int16_t)m.height) endY = m.height;
    for (; y < endY; ++y) hline(x, y, w, c);
  }
  void bar(uint16_t x, uint16_t w, uint16_t level, Color c) const {
    if (level > m.height) level = m.height;
    fillRect(x, m.height - level, w, level, c);
  }
};
