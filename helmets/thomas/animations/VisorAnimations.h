#pragma once
// Thomas-only static classic visor: colored bands from top to bottom.
// The nine colors fill each visor independently in its logical 4x10 geometry.
#include "LEDLayouts.h"

template <class Layout>
inline void classicVisorPanel() {
  static_assert(Layout::isMatrix, "Classic Thomas visor needs a matrix layout");
  constexpr Color bands[8] = {
    Purple, Purple, Cyan, Green, Yellow, Yellow, Red, Yellow
  };
  for (uint16_t y = 0; y < Layout::height; ++y) {
    const uint8_t band = uint8_t((uint32_t(y) * 8u) / Layout::height);
    for (uint16_t x = 0; x < Layout::width; ++x)
      Layout::setXY(x, y, bands[band]);
  }
}