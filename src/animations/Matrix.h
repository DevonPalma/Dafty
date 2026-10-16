#pragma once
// True 2D effects. Layout::setXY() is specialized by LEDLayouts.h.
// No graphics wrapper, frame buffers, or dynamic allocation.
#include "Common.h"

// A column of light moving left/right with a small glow trail.
// Kept under the name 'scanner' for compatibility with existing states.
template <class Layout>
inline void scanner(Color color, uint32_t elapsedMs,
                    uint16_t periodMs = 2200, uint8_t trail = 2)
{
  static_assert(Layout::isMatrix, "scanner requires a Matrix layout");
  if (!Layout::width || periodMs < 2)
    return;
  const uint16_t half = periodMs / 2;
  const uint16_t phase = elapsedMs % periodMs;
  const uint16_t travel = Layout::width - 1;
  const uint16_t position = phase <= half ? phase : periodMs - phase;
  const uint16_t head = uint16_t((uint32_t(position) * travel) / half);
  for (int16_t dx = -int16_t(trail); dx <= int16_t(trail); ++dx)
  {
    const int16_t x = int16_t(head) + dx;
    if (x < 0 || x >= int16_t(Layout::width))
      continue;
    const uint8_t distance = uint8_t(dx < 0 ? -dx : dx);
    const uint8_t glow = uint8_t(255u - uint16_t(distance) * 255u / (uint16_t(trail) + 1u));
    const Color shaded = anim::dim(color, glow);
    for (uint16_t y = 0; y < Layout::height; ++y)
      Layout::setXY(uint16_t(x), y, shaded);
  }
}

template <class Layout>
inline void matrixScanner(Color color, uint32_t elapsedMs,
                          uint16_t periodMs = 2200, uint8_t trail = 2)
{
  scanner<Layout>(color, elapsedMs, periodMs, trail);
}

// 16 narrow bars on a 48-column panel by default. Heights blend smoothly
// between deterministic beats. 'periodMs' controls one repeating animation.
// No audio input is involved; this is a synthesized visualizer.
template <class Layout>
inline void matrixEqualizer(Color low, Color high, uint32_t elapsedMs,
                            uint16_t periodMs = 4000, uint8_t bars = 16,
                            uint8_t beats = 16)
{
  static_assert(Layout::isMatrix, "matrixEqualizer requires a Matrix layout");
  if (!periodMs || !Layout::width || !Layout::height || !bars)
    return;
  if (bars > Layout::width)
    bars = Layout::width;
  if (!beats)
    beats = 1;
  const uint32_t progress = uint32_t(elapsedMs % periodMs) * beats;
  const uint16_t beat = progress / periodMs;
  const uint8_t blendPhase = uint8_t(((progress % periodMs) * 255UL) / periodMs);
  const uint16_t height = Layout::height;
  for (uint8_t bar = 0; bar < bars; ++bar)
  {
    // One pseudo-random height calculation per bar, not per pixel.
    const uint16_t h1 = 1u + anim::hash16(uint16_t(bar * 97u + beat * 211u)) % height;
    const uint16_t h2 = 1u + anim::hash16(uint16_t(bar * 97u + ((beat + 1u) % beats) * 211u)) % height;
    const uint16_t level = uint16_t((uint32_t(h1) * (255 - blendPhase) +
                                     uint32_t(h2) * blendPhase + 127) /
                                    255);
    const uint16_t startX = uint32_t(bar) * Layout::width / bars;
    const uint16_t stopX = uint32_t(bar + 1u) * Layout::width / bars;
    for (uint16_t x = startX; x < stopX; ++x)
    {
      // Leave a one-column gap only for bars wider than two columns.
      if (stopX - startX > 2 && x == stopX - 1)
        continue;
      for (uint16_t n = 0; n < level; ++n)
      {
        const uint16_t y = height - 1 - n;
        const uint8_t mix = height > 1 ? uint8_t(uint32_t(n) * 255u / (height - 1)) : 0;
        Layout::setXY(x, y, anim::blend(low, high, mix));
      }
    }
  }
}

// 2D vertical digital-rain trails. drops controls simultaneous columns.
template <class Layout>
inline void matrixRain(Color color, uint32_t elapsedMs, uint16_t stepMs = 130,
                       uint8_t drops = 12, uint8_t trail = 3)
{
  static_assert(Layout::isMatrix, "matrixRain requires a Matrix layout");
  if (!stepMs || !Layout::width || !Layout::height)
    return;
  const uint16_t tick = elapsedMs / stepMs;
  for (uint8_t d = 0; d < drops; ++d)
  {
    const uint16_t seed = anim::hash16(uint16_t(d * 251u + 17u));
    const uint16_t x = seed % Layout::width;
    const uint16_t run = uint16_t(Layout::height + trail + 1u);
    const uint16_t head = (uint32_t(tick / (1u + d % 3u)) + seed / 7u) % run;
    for (uint8_t t = 0; t <= trail; ++t)
    {
      const int16_t y = int16_t(head) - t;
      if (y < 0 || y >= int16_t(Layout::height))
        continue;
      const uint8_t brightness = uint8_t(255u - uint16_t(t) * 255u / (uint16_t(trail) + 1u));
      Layout::setXY(x, uint16_t(y), anim::dim(color, brightness));
    }
  }
}

// Random-looking small rectangular disruptions; few sparse rectangles per step.
template <class Layout>
inline void matrixGlitch(Color base, Color flash, uint32_t elapsedMs,
                         uint16_t stepMs = 90, uint8_t blocks = 10)
{
  static_assert(Layout::isMatrix, "matrixGlitch requires a Matrix layout");
  if (!stepMs || !Layout::width || !Layout::height)
    return;
  const uint16_t tick = elapsedMs / stepMs;
  for (uint8_t b = 0; b < blocks; ++b)
  {
    const uint16_t seed = anim::hash16(uint16_t(tick * 151u + b * 443u));
    const uint16_t x = seed % Layout::width;
    const uint16_t y = (seed >> 5) % Layout::height;
    const uint8_t w = uint8_t(1 + (seed >> 9) % 7);
    const uint8_t h = uint8_t(1 + (seed >> 13) % 3);
    const Color color = (seed & 3u) == 0 ? flash : anim::dim(base, 100);
    for (uint8_t dy = 0; dy < h && y + dy < Layout::height; ++dy)
      for (uint8_t dx = 0; dx < w && x + dx < Layout::width; ++dx)
        Layout::setXY(x + dx, y + dy, color);
  }
}

// Horizontal rainbow across X. Every column has the same color across Y.
template <class Layout>
inline void matrixRainbow(uint32_t elapsedMs, uint16_t periodMs = 3500)
{
  static_assert(Layout::isMatrix, "matrixRainbow requires a Matrix layout");
  if (!periodMs || !Layout::width)
    return;
  const uint8_t shift = anim::phase8(elapsedMs, periodMs);
  for (uint16_t x = 0; x < Layout::width; ++x)
  {
    const Color color = anim::wheel(uint8_t(uint32_t(x) * 256UL / Layout::width + shift));
    for (uint16_t y = 0; y < Layout::height; ++y)
      Layout::setXY(x, y, color);
  }
}

// Traveling undulating line. Integer triangle-wave math keeps AVR cost low.
template <class Layout>
inline void matrixWave(Color color, uint32_t elapsedMs, uint16_t periodMs = 2400,
                       uint8_t cycles = 2)
{
  static_assert(Layout::isMatrix, "matrixWave requires a Matrix layout");
  if (!periodMs || !Layout::width || !Layout::height)
    return;
  const uint8_t shift = anim::phase8(elapsedMs, periodMs);
  for (uint16_t x = 0; x < Layout::width; ++x)
  {
    const uint8_t phase = uint8_t((uint32_t(x) * cycles * 256UL) / Layout::width + shift);
    const uint8_t tri = phase < 128 ? uint8_t(phase * 2) : uint8_t((255 - phase) * 2);
    const uint16_t y = (uint32_t(tri) * (Layout::height - 1)) / 255;
    Layout::setXY(x, y, color);
  }
}
