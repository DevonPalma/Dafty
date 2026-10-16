#pragma once
// Linear animations. Usable with Strip, Ring, Group, or Matrix layouts because
// every layout exposes ::count and ::set(logicalIndex, color).
#include "Common.h"

// Every LED in the selected layout displays a constant color.
template<class Layout>
inline void solid(Color color) {
  for (uint16_t i = 0; i < Layout::count; ++i) Layout::set(i, color);
}

// Gradually gets brighter then dimmer, once per periodMs.
template<class Layout>
inline void pulse(Color color, uint32_t elapsedMs, uint16_t periodMs = 1800) {
  if (periodMs < 2) return;
  solid<Layout>(anim::dim(color, anim::triangle(elapsedMs, periodMs)));
}

// A -> B color transition, repeated each period. Transitions snap back to A.
template<class Layout>
inline void fade(Color from, Color to, uint32_t elapsedMs, uint16_t periodMs = 2000) {
  if (!periodMs) return;
  solid<Layout>(anim::blend(from, to, anim::phase8(elapsedMs, periodMs)));
}

// Fill LEDs in logical layout order. Entire layout fills by end of the period.
template<class Layout>
inline void wipe(Color color, uint32_t elapsedMs, uint16_t periodMs = 1800) {
  if (!periodMs) return;
  const uint16_t lit = (uint32_t(elapsedMs % periodMs) * Layout::count) / periodMs;
  for (uint16_t i = 0; i < lit; ++i) Layout::set(i, color);
}

// Point with a fading tail; logical order wraps around the selected layout.
template<class Layout>
inline void comet(Color color, uint32_t elapsedMs, uint16_t periodMs = 2200,
                  uint8_t tail = 6) {
  if (!periodMs || !Layout::count || !tail) return;
  const uint16_t head = (uint32_t(elapsedMs % periodMs) * Layout::count) / periodMs;
  for (uint8_t step = 0; step < tail && step < Layout::count; ++step) {
    const uint16_t index = (uint32_t(head) + Layout::count - step) % Layout::count;
    Layout::set(index, anim::dim(color, uint8_t(255u - (uint16_t(step) * 255u) / tail)));
  }
}

// Rainbow along the logical one-dimensional layout order.
template<class Layout>
inline void rainbow(Color /*unused*/, uint32_t elapsedMs, uint16_t periodMs = 3000) {
  if (!periodMs || !Layout::count) return;
  const uint8_t shift = anim::phase8(elapsedMs, periodMs);
  for (uint16_t i = 0; i < Layout::count; ++i) {
    const uint8_t hue = uint8_t((uint32_t(i) * 256UL) / Layout::count + shift);
    Layout::set(i, anim::wheel(hue));
  }
}

// Convenience overload without a color argument.
template<class Layout>
inline void rainbow(uint32_t elapsedMs, uint16_t periodMs = 3000) {
  rainbow<Layout>(White, elapsedMs, periodMs);
}
