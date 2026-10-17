#pragma once
// Guy Manuel-only animations. Like shared effects, they draw pixels from elapsedMs.
// No knowledge of commands, states, BLE, or the current helmet mode.
#include "LEDLayouts.h"
#include "Animations.h"
#include "TextAnimations.h"

template<class Layout>
inline void workIt(uint32_t elapsedMs) {
  const char* phrase = nullptr;
  Color ink = Cyan;
  uint32_t offset = 0;
  constexpr uint32_t STEP = 1800;
  if (elapsedMs < STEP) { phrase = "WORK IT"; ink = Cyan; }
  else if (elapsedMs < STEP * 2UL) { phrase = "MAKE IT"; ink = Yellow; offset = STEP; }
  else if (elapsedMs < STEP * 3UL) { phrase = "DO IT"; ink = Purple; offset = STEP * 2UL; }
  else { phrase = "HARDER"; ink = Red; offset = STEP * 3UL; }
  matrixCenteredText<Layout>(phrase, ink);
  // Quick pulse between phrases, with the current state clock.
  if (((elapsedMs - offset) / 180UL) % 2UL)
    matrixGlitch<Layout>(Black, ink, elapsedMs, 110, 3);
}


// Two animated heart-shaped eyes on the 48x8 matrix. No extra framebuffer.
template<class Layout>
inline void heartEyes(uint32_t elapsedMs) {
  static const char* const rows[8] = {
    "..##...##..",
    ".####.####.",
    "###########",
    ".#########.",
    "..#######..",
    "...#####...",
    "....###....",
    ".....#....."
  };
  // Alternate between a soft and bright pink/red heartbeat.
  const uint16_t beat = uint16_t(elapsedMs % 1300UL);
  const uint8_t power = (beat < 160 || (beat >= 230 && beat < 390)) ? 255 : 105;
  const Color heart = anim::dim(RGB(255, 22, 80), power);
  for (uint8_t y = 0; y < 8; ++y) {
    for (uint8_t x = 0; x < 11; ++x) {
      if (rows[y][x] == '#') {
        Layout::setXY(7 + x, y, heart);
        Layout::setXY(30 + x, y, heart);
      }
    }
  }
  // Two little white glints during the brighter heartbeat.
  if (power == 255) {
    Layout::setXY(9, 1, White);
    Layout::setXY(32, 1, White);
  }
}
