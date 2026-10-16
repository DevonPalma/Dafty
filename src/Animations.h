#pragma once
// Animation algorithms. Layout is a C++ template parameter, so matrix
// dimensions, serpentine wiring, start offsets and reversals are fixed at build.
// Each renderer writes to Adafruit_NeoPixel's existing RGB buffer.
#include <Arduino.h>
#include <math.h>
#include "AnimationTypes.h"

inline uint8_t clampByte(int v) { return v < 0 ? 0 : (v > 255 ? 255 : v); }
inline float clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }
inline Color mixColor(Color a, Color b, uint8_t alpha)
{
  return {(uint8_t)((a.r * (uint16_t)(255 - alpha) + b.r * (uint16_t)alpha) / 255),
          (uint8_t)((a.g * (uint16_t)(255 - alpha) + b.g * (uint16_t)alpha) / 255),
          (uint8_t)((a.b * (uint16_t)(255 - alpha) + b.b * (uint16_t)alpha) / 255)};
}
inline Color dimColor(Color c, uint8_t amount)
{
  return {(uint8_t)(c.r * (uint16_t)amount / 255),
          (uint8_t)(c.g * (uint16_t)amount / 255),
          (uint8_t)(c.b * (uint16_t)amount / 255)};
}
inline uint16_t lightHash(uint16_t v)
{
  v ^= v >> 7;
  v *= 40503u;
  v ^= v >> 5;
  return v;
}
inline Color wheel(uint8_t x)
{
  if (x < 85)
    return {(uint8_t)(255 - x * 3), (uint8_t)(x * 3), 0};
  if (x < 170)
  {
    x -= 85;
    return {0, (uint8_t)(255 - x * 3), (uint8_t)(x * 3)};
  }
  x -= 170;
  return {(uint8_t)(x * 3), 0, (uint8_t)(255 - x * 3)};
}

template <typename L>
inline void fillLayout(Color c)
{
  for (uint16_t i = 0; i < L::count; ++i)
    L::set(i, c);
}

template <typename L>
inline void drawComet(Color color, uint16_t head, uint8_t trail, bool reverse)
{
  const uint16_t n = L::count;
  if (!n)
    return;
  if (!trail)
    trail = 1;
  for (uint8_t t = 0; t < trail; ++t)
  {
    const uint16_t d = t % n;
    uint16_t i = reverse ? (head + d) % n : (head + n - d) % n;
    L::set(i, dimColor(color, (uint8_t)(255u * (trail - t) / trail)));
  }
}

// Matrix versions draw in X/Y coordinates. Non-matrix layouts retain 1D logic.
// The compiler removes the unused branch for each instantiated layout.
template <typename L>
inline void renderEffect(const FlashEffect &e, float p)
{
  const FlashParams &cfg = e.parameters;
  const uint8_t phase = (uint8_t)(clamp01(p) * 255.0f);
  switch (e.kind)
  {
  case EffectKind::Solid:
    fillLayout<L>(cfg.a);
    break;
  case EffectKind::Fade:
    fillLayout<L>(mixColor(cfg.a, cfg.b, phase));
    break;
  case EffectKind::Pulse:
  {
    uint8_t intensity = phase < 128 ? phase * 2 : (255 - phase) * 2;
    fillLayout<L>(dimColor(cfg.a, intensity));
    break;
  }
  case EffectKind::Wipe:
  {
    uint16_t n = (uint32_t)phase * L::count / 255u;
    for (uint16_t i = 0; i < n; ++i)
      L::set(i, cfg.a);
    break;
  }
  case EffectKind::Rainbow:
  {
    const int16_t shift = (int16_t)(p * cfg.f1 * 255.0f) * (cfg.flag ? -1 : 1);
    if (L::isMatrix)
    {
      for (uint16_t x = 0; x < L::width; ++x)
      {
        Color c = wheel((uint8_t)((uint32_t)x * 256u / L::width + shift));
        for (uint16_t y = 0; y < L::height; ++y)
          L::setXY(x, y, c);
      }
    }
    else
    {
      for (uint16_t i = 0; i < L::count; ++i)
        L::set(i, wheel((uint8_t)((uint32_t)i * 256u / L::count + shift)));
    }
    break;
  }
  case EffectKind::Comet:
  {
    if (!L::count)
      break;
    uint16_t h1 = (uint16_t)(p * cfg.f1 * L::count) % L::count;
    uint16_t h2 = (uint16_t)((1.0f - p) * cfg.f2 * L::count) % L::count;
    drawComet<L>(cfg.a, h1, cfg.n1, false);
    drawComet<L>(cfg.b, h2, cfg.n1, true);
    break;
  }
  case EffectKind::Equalizer:
  {
    const uint16_t extent = L::isMatrix ? L::width : L::count;
    uint16_t bars = cfg.n1 ? cfg.n1 : 1;
    if (bars > extent)
      bars = extent;
    const uint16_t beat = (uint16_t)(p * (cfg.n2 ? cfg.n2 : 1));
    for (uint16_t bar = 0; bar < bars; ++bar)
    {
      const uint16_t begin = (uint32_t)bar * extent / bars;
      const uint16_t end = (uint32_t)(bar + 1) * extent / bars;
      if (L::isMatrix)
      {
        // One height per bar, not one hash per pixel. 8-high matrix.
        const uint16_t height = 1 + lightHash(bar * 73u + beat * 101u) % L::height;
        for (uint16_t y = 0; y < height; ++y)
        {
          Color c = mixColor(cfg.a, cfg.b, (uint32_t)y * 255u / L::height);
          for (uint16_t x = begin; x < end; ++x)
            L::setXY(x, L::height - 1 - y, c);
        }
      }
      else
      {
        const uint16_t width = end - begin;
        if (!width)
          continue;
        const uint16_t height = 1 + lightHash(bar * 73u + beat * 101u) % width;
        Color c = mixColor(cfg.a, cfg.b, (uint32_t)height * 255u / width);
        for (uint16_t i = 0; i < height; ++i)
          L::set(begin + i, c);
      }
    }
    break;
  }
  case EffectKind::Scanner:
  {
    float f = p * (cfg.n2 ? cfg.n2 : 1);
    f -= floorf(f);
    const float triangle = f < 0.5f ? 2 * f : 2 - 2 * f;
    const uint16_t extent = L::isMatrix ? L::width : L::count;
    if (!extent)
      break;
    int16_t head = (int16_t)(triangle * (extent - 1) + 0.5f);
    int16_t radius = cfg.n1 ? cfg.n1 : 1;
    for (int16_t d = -radius; d <= radius; ++d)
    {
      int16_t col = head + d;
      if (col < 0 || col >= extent)
        continue;
      uint8_t bright = (uint32_t)(radius + 1 - abs(d)) * 255u / (radius + 1);
      Color c = dimColor(cfg.a, (uint16_t)bright * bright / 255u);
      if (L::isMatrix)
      {
        for (uint16_t y = 0; y < L::height; ++y)
          L::setXY(col, y, c);
      }
      else
        L::set(col, c);
    }
    break;
  }
  case EffectKind::Glitch:
  {
    uint16_t blocks = cfg.n1 ? cfg.n1 : 1;
    const uint16_t tick = (uint16_t)(p * (cfg.n2 ? cfg.n2 : 1));
    if (L::isMatrix)
    {
      // Scattered 2D patches, positioned deterministically each tick.
      for (uint16_t b = 0; b < blocks; ++b)
      {
        uint16_t h = lightHash(b * 313u + tick * 887u);
        uint16_t x = h % L::width, y = (h >> 7) % L::height;
        uint16_t w = 1 + (h >> 4) % 5, hh = 1 + (h >> 11) % 3;
        Color c = (h % 5 == 0) ? cfg.b : dimColor(cfg.a, 110);
        for (uint16_t iy = y; iy < y + hh && iy < L::height; ++iy)
          for (uint16_t ix = x; ix < x + w && ix < L::width; ++ix)
            L::setXY(ix, iy, c);
      }
    }
    else
    {
      if (blocks > L::count)
        blocks = L::count;
      for (uint16_t b = 0; b < blocks; ++b)
      {
        uint16_t h = lightHash(b * 313u + tick * 887u);
        Color c = (h % 5 == 0) ? cfg.b : dimColor(cfg.a, 110);
        for (uint16_t i = (uint32_t)b * L::count / blocks; i < (uint32_t)(b + 1) * L::count / blocks; ++i)
          L::set(i, c);
      }
    }
    break;
  }
  case EffectKind::NeonSteps:
  {
    uint8_t w = cfg.n1 ? cfg.n1 : 1;
    uint16_t offset = (uint16_t)(p * (cfg.n2 ? cfg.n2 : 1) * w * 3u);
    if (L::isMatrix)
    {
      for (uint16_t x = 0; x < L::width; ++x)
      {
        uint8_t slot = ((x + offset) / w) % 3;
        if (slot == 2)
          continue;
        for (uint16_t y = 0; y < L::height; ++y)
          L::setXY(x, y, slot == 0 ? cfg.a : cfg.b);
      }
    }
    else
    {
      for (uint16_t i = 0; i < L::count; ++i)
      {
        uint8_t slot = ((i + offset) / w) % 3;
        if (slot < 2)
          L::set(i, slot == 0 ? cfg.a : cfg.b);
      }
    }
    break;
  }
  case EffectKind::MatrixDot:
  {
    if (!L::isMatrix)
      break;
    uint16_t i = (uint32_t)phase * L::count / 255u;
    if (i >= L::count)
      i = L::count - 1;
    L::setXY(i % L::width, i / L::width, cfg.a);
    break;
  }
  case EffectKind::RingChase:
  {
    uint16_t head = (uint32_t)phase * L::count / 256u;
    drawComet<L>(cfg.a, head, 4, false);
    break;
  }
  case EffectKind::MatrixTest:
  {
    if (!L::isMatrix)
      break;
    if (p < 0.25f)
    {
      L::setXY(0, 0, Red);
      L::setXY(L::width - 1, 0, Green);
      L::setXY(0, L::height - 1, Blue);
      L::setXY(L::width - 1, L::height - 1, White);
    }
    else if (p < 0.5f)
    {
      uint16_t y = (uint16_t)((p - 0.25f) * 4 * L::height);
      if (y >= L::height)
        y = L::height - 1;
      for (uint16_t x = 0; x < L::width; ++x)
        L::setXY(x, y, Yellow);
    }
    else if (p < 0.75f)
    {
      uint16_t x = (uint16_t)((p - 0.5f) * 4 * L::width);
      if (x >= L::width)
        x = L::width - 1;
      for (uint16_t y = 0; y < L::height; ++y)
        L::setXY(x, y, Cyan);
    }
    else
    {
      // Raw physical groups, intentionally bypass matrix mapping.
      uint16_t groups = (L::count + 7) / 8;
      uint16_t g = (uint16_t)((p - 0.75f) * 4 * groups);
      if (g >= groups)
        g = groups - 1;
      for (uint16_t i = g * 8; i < g * 8 + 8 && i < L::count; ++i)
        writePhysical(L::start + i, Purple);
    }
    break;
  }
  }
}
