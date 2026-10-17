#pragma once
// Guy Manuel-only animations. Like shared effects, they draw pixels from elapsedMs.
// No knowledge of commands, states, BLE, or the current helmet mode.
#include "../LEDLayouts.h"
#include "Animations.h"
#include "TextAnimations.h"

template <class Layout>
inline void workIt(uint32_t elapsedMs)
{
  const char *phrase = nullptr;
  Color ink = Cyan;
  uint32_t offset = 0;
  constexpr uint32_t STEP = 1800;
  if (elapsedMs < STEP)
  {
    phrase = "WORK IT";
    ink = Cyan;
  }
  else if (elapsedMs < STEP * 2UL)
  {
    phrase = "MAKE IT";
    ink = Yellow;
    offset = STEP;
  }
  else if (elapsedMs < STEP * 3UL)
  {
    phrase = "DO IT";
    ink = Purple;
    offset = STEP * 2UL;
  }
  else
  {
    phrase = "HARDER";
    ink = Red;
    offset = STEP * 3UL;
  }
  matrixCenteredText<Layout>(phrase, ink);
  // Quick pulse between phrases, with the current state clock.
  if (((elapsedMs - offset) / 180UL) % 2UL)
    matrixGlitch<Layout>(Black, ink, elapsedMs, 110, 3);
}

// Two animated heart-shaped eyes on the 48x8 matrix. No extra framebuffer.
template <class Layout>
inline void heartEyes(uint32_t elapsedMs)
{
  static const char *const rows[8] = {
      "..##...##..",
      ".####.####.",
      "###########",
      ".#########.",
      "..#######..",
      "...#####...",
      "....###....",
      ".....#....."};
  // Alternate between a soft and bright pink/red heartbeat.
  const uint16_t beat = uint16_t(elapsedMs % 1300UL);
  const uint8_t power = (beat < 160 || (beat >= 230 && beat < 390)) ? 255 : 105;
  const Color heart = anim::dim(RGB(255, 22, 80), power);
  for (uint8_t y = 0; y < 8; ++y)
  {
    for (uint8_t x = 0; x < 11; ++x)
    {
      if (rows[y][x] == '#')
      {
        Layout::setXY(7 + x, y, heart);
        Layout::setXY(30 + x, y, heart);
      }
    }
  }
  // Two little white glints during the brighter heartbeat.
  if (power == 255)
  {
    Layout::setXY(9, 1, White);
    Layout::setXY(32, 1, White);
  }
}

// Expressions below are strictly drawing functions. They have no knowledge of
// BLE, command IDs, destinations, state durations, or button input.
namespace guyFx
{
  template <class Layout>
  inline void eyeLine(uint8_t x, uint8_t y, Color c)
  {
    for (uint8_t dx = 0; dx < 12; ++dx)
      Layout::setXY(x + dx, y, c);
  }
  template <class Layout>
  inline void eye(uint8_t x, uint32_t elapsedMs, bool closed, Color c)
  {
    if (closed)
    {
      eyeLine<Layout>(x, 4, c);
      return;
    }
    // 12x5 rounded eye with two bright pupil pixels.
    for (uint8_t dx = 1; dx < 11; ++dx)
    {
      Layout::setXY(x + dx, 1, c);
      Layout::setXY(x + dx, 6, c);
    }
    for (uint8_t y = 2; y < 6; ++y)
    {
      Layout::setXY(x, y, c);
      Layout::setXY(x + 11, y, c);
    }
    const uint8_t pupil = 4 + uint8_t((elapsedMs / 900) % 3);
    Layout::setXY(x + pupil, 3, White);
    Layout::setXY(x + pupil + 1, 4, White);
  }
}

template <class Layout>
inline void blinkEyes(uint32_t elapsedMs)
{
  static_assert(Layout::isMatrix, "Eyes need a matrix");
  const bool shut = elapsedMs >= 360 && elapsedMs < 610;
  guyFx::eye<Layout>(5, elapsedMs, shut, Cyan);
  guyFx::eye<Layout>(31, elapsedMs, shut, Cyan);
}

template <class Layout>
inline void winkEye(uint32_t elapsedMs)
{
  const bool shut = elapsedMs >= 350 && elapsedMs < 1000;
  guyFx::eye<Layout>(5, elapsedMs, false, Cyan);
  guyFx::eye<Layout>(31, elapsedMs, shut, Cyan);
}

template <class Layout>
inline void angryEyes(uint32_t elapsedMs)
{
  const Color c = anim::dim(Red, uint8_t(155 + 100u * anim::triangle(elapsedMs, 1600) / 255u));
  // Inward-downward eyebrows; two glowing pupils beneath them.
  for (uint8_t x = 0; x < 15; ++x)
  {
    Layout::setXY(5 + x, 1 + x / 5, c);
    Layout::setXY(28 + x, 3 - x / 5, c);
  }
  for (uint8_t y = 4; y < 7; ++y)
  {
    for (uint8_t x = 9; x < 17; ++x)
      Layout::setXY(x, y, c);
    for (uint8_t x = 32; x < 40; ++x)
      Layout::setXY(x, y, c);
  }
  Layout::setXY(13, 5, White);
  Layout::setXY(35, 5, White);
}

template <class Layout>
inline void heartbeat(uint32_t elapsedMs)
{
  const uint8_t strength = anim::triangle(elapsedMs, 1000);
  const Color c = anim::dim(Cyan, uint8_t(90u + uint16_t(strength) * 165u / 255u));
  const uint8_t heights[12] = {4, 4, 4, 2, 6, 0, 7, 3, 4, 4, 4, 4};
  for (uint8_t x = 0; x < Layout::width; ++x)
  {
    const uint8_t y = heights[(x + elapsedMs / 95) % 12];
    Layout::setXY(x, y, c);
    if (y < 7)
      Layout::setXY(x, y + 1, anim::dim(c, 55));
  }
}

template <class Layout>
inline void glitchFace(uint32_t elapsedMs)
{
  matrixGlitch<Layout>(Black, Purple, elapsedMs, 65, 15);
  matrixScanner<Layout>(White, elapsedMs, 300, 1);
}

template <class Layout>
inline void sleepingEyes(uint32_t elapsedMs)
{
  const bool brieflyOpen = (elapsedMs % 7000UL) < 200;
  guyFx::eye<Layout>(5, elapsedMs, !brieflyOpen, Blue);
  guyFx::eye<Layout>(31, elapsedMs, !brieflyOpen, Blue);
}

template <class Layout>
inline void loadingFace(uint32_t elapsedMs)
{
  const uint8_t step = uint8_t((elapsedMs / 180UL) % 12);
  for (uint8_t i = 0; i < 12; ++i)
  {
    const uint8_t intensity = uint8_t(i == step ? 255 : (i + 12 - step) % 12 < 4 ? 80
                                                                                 : 12);
    for (uint8_t y = 2; y < 6; ++y)
      Layout::setXY(6 + i * 3, y, anim::dim(Cyan, intensity));
  }
}

template <class Layout>
inline void greenHeart(uint32_t elapsedMs)
{
  static_assert(Layout::isMatrix, "Requires a matrix");
  (void)elapsedMs; // Static animation

  // Green gradient: bright edges, black center.
  for (uint16_t y = 0; y < Layout::height; ++y)
  {
    for (uint16_t x = 0; x < Layout::width; ++x)
    {
      const uint16_t center = Layout::width / 2;
      const uint16_t distance =
          x < center ? center - x : x - center;

      const uint8_t green = uint8_t(
          uint32_t(distance) * 255 / center);

      Layout::setXY(x, y, RGB(0, green, 0));
    }
  }

  // 11x7 pixel heart.
  static const uint16_t heart[7] = {
      0x18C,
      0x3DE,
      0x7FF,
      0x3FE,
      0x1FC,
      0x0F8,
      0x070};

  const uint16_t startX = (Layout::width - 11) / 2 + 1;
  const uint16_t startY = (Layout::height - 7) / 2;

  for (uint8_t y = 0; y < 7; ++y)
  {
    for (uint8_t x = 0; x < 11; ++x)
    {
      if (heart[y] & (uint16_t(1) << x))
      {
        Layout::setXY(
            startX + x,
            startY + y,
            RGB(255, 0, 0));
      }
    }
  }
}