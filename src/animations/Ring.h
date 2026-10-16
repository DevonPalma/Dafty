#pragma once
// Effects designed for circular LED layouts. Set only the pixels that are lit.
#include "Common.h"

template <class Layout>
inline void ringChase(Color color, uint32_t elapsedMs, uint16_t periodMs = 1500,
                      uint8_t tail = 4)
{
  if (!periodMs || !Layout::count || !tail)
    return;
  const uint16_t head = (uint32_t(elapsedMs % periodMs) * Layout::count) / periodMs;
  for (uint8_t d = 0; d < tail && d < Layout::count; ++d)
  {
    const uint16_t i = (uint32_t(head) + Layout::count - d) % Layout::count;
    Layout::set(i, anim::dim(color, uint8_t(255u - uint16_t(d) * 255u / tail)));
  }
}

// Opposing light heads travel around a ring, each with a short tail.
template <class Layout>
inline void ringDualChase(Color first, Color second, uint32_t elapsedMs,
                          uint16_t periodMs = 1800, uint8_t tail = 3)
{
  if (!periodMs || !Layout::count || !tail)
    return;
  const uint16_t head = (uint32_t(elapsedMs % periodMs) * Layout::count) / periodMs;
  for (uint8_t d = 0; d < tail && d < Layout::count; ++d)
  {
    const uint8_t glow = uint8_t(255u - uint16_t(d) * 255u / tail);
    Layout::set((uint32_t(head) + Layout::count - d) % Layout::count,
                anim::dim(first, glow));
    Layout::set((uint32_t(Layout::count - 1 - head) + d) % Layout::count,
                anim::dim(second, glow));
  }
}

// Circular breathing using triangle-wave brightness.
template <class Layout>
inline void ringBreathe(Color color, uint32_t elapsedMs, uint16_t periodMs = 1800)
{
  if (periodMs < 2)
    return;
  const uint8_t brightness = anim::triangle(elapsedMs, periodMs);
  for (uint16_t i = 0; i < Layout::count; ++i)
    Layout::set(i, anim::dim(color, brightness));
}

// A few changing sparkles, deterministic and without an extra pixel buffer.
template <class Layout>
inline void ringSparkle(Color color, uint32_t elapsedMs, uint16_t stepMs = 150,
                        uint8_t sparks = 3)
{
  if (!Layout::count || !stepMs)
    return;
  const uint16_t tick = elapsedMs / stepMs;
  for (uint8_t s = 0; s < sparks; ++s)
  {
    const uint16_t n = anim::hash16(uint16_t(tick * 79u + s * 317u));
    Layout::set(n % Layout::count, anim::dim(color, uint8_t(110u + n % 146u)));
  }
}
