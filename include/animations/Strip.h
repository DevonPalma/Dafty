#pragma once
// Linear animations. Usable with Strip, Ring, Group, or Matrix layouts because
// every layout exposes ::count and ::set(logicalIndex, color).
#include "Common.h"

// Every LED in the selected layout displays a constant color.
template <class Layout>
inline void solid(Color color)
{
  for (uint16_t i = 0; i < Layout::count; ++i)
    Layout::set(i, color);
}

// Gradually gets brighter then dimmer, once per periodMs.
template <class Layout>
inline void pulse(Color color, uint32_t elapsedMs, uint16_t periodMs = 1800)
{
  if (periodMs < 2)
    return;
  solid<Layout>(anim::dim(color, anim::triangle(elapsedMs, periodMs)));
}

// A -> B color transition, repeated each period. Transitions snap back to A.
template <class Layout>
inline void fade(Color from, Color to, uint32_t elapsedMs, uint16_t periodMs = 2000)
{
  if (!periodMs)
    return;
  solid<Layout>(anim::blend(from, to, anim::phase8(elapsedMs, periodMs)));
}

// Fill LEDs in logical layout order. Entire layout fills by end of the period.
template <class Layout>
inline void wipe(Color color, uint32_t elapsedMs, uint16_t periodMs = 1800)
{
  if (!periodMs)
    return;
  const uint16_t lit = (uint32_t(elapsedMs % periodMs) * Layout::count) / periodMs;
  for (uint16_t i = 0; i < lit; ++i)
    Layout::set(i, color);
}

// Point with a fading tail; logical order wraps around the selected layout.
template <class Layout>
inline void comet(Color color, uint32_t elapsedMs, uint16_t periodMs = 2200,
                  uint8_t tail = 6)
{
  if (!periodMs || !Layout::count || !tail)
    return;
  const uint16_t head = (uint32_t(elapsedMs % periodMs) * Layout::count) / periodMs;
  for (uint8_t step = 0; step < tail && step < Layout::count; ++step)
  {
    const uint16_t index = (uint32_t(head) + Layout::count - step) % Layout::count;
    Layout::set(index, anim::dim(color, uint8_t(255u - (uint16_t(step) * 255u) / tail)));
  }
}

// Rainbow along the logical one-dimensional layout order.
template <class Layout>
inline void rainbow(Color /*unused*/, uint32_t elapsedMs, uint16_t periodMs = 3000)
{
  if (!periodMs || !Layout::count)
    return;
  const uint8_t shift = anim::phase8(elapsedMs, periodMs);
  for (uint16_t i = 0; i < Layout::count; ++i)
  {
    const uint8_t hue = uint8_t((uint32_t(i) * 256UL) / Layout::count + shift);
    Layout::set(i, anim::wheel(hue));
  }
}

// Convenience overload without a color argument.
template <class Layout>
inline void rainbow(uint32_t elapsedMs, uint16_t periodMs = 3000)
{
  rainbow<Layout>(White, elapsedMs, periodMs);
}


template<class Layout>
inline void twinkle(
    Color base,
    Color sparkle,
    uint32_t elapsedMs,
    uint16_t periodMs = 2000
) {
    // Each window gets a different randomized pattern.
    const uint32_t window = elapsedMs / periodMs;
    const uint16_t phase = elapsedMs % periodMs;

    for (uint16_t i = 0; i < Layout::count; ++i) {

        // Randomized timing for each LED and each window.
        const uint16_t seed = anim::hash16(
            uint16_t(i * 317u) ^ uint16_t(window * 7919u)
        );

        // Occasionally skip a sparkle entirely.
        if ((seed & 3u) == 0) {
            Layout::set(i, base);
            continue;
        }

        // Start somewhere in the time window.
        const uint16_t start =
            uint16_t(seed % periodMs);

        // Short flare followed by a longer fade.
        const uint16_t glowMs = periodMs / 4;
        const uint16_t attackMs = glowMs / 4;

        uint8_t brightness = 0;

        // Only draw after the randomized start time.
        if (glowMs > 0 && attackMs > 0 &&
            phase >= start && phase - start < glowMs) {

            const uint16_t t = phase - start;

            if (t < attackMs) {
                const uint32_t x =
                    uint32_t(t) * 255u / attackMs;

                brightness = uint8_t(x * x / 255u);
            }
            else {
                const uint16_t fadeMs = glowMs - attackMs;
                const uint32_t x =
                    uint32_t(glowMs - t) * 255u / fadeMs;

                brightness = uint8_t(x * x / 255u);
            }
        }

        Layout::set(
            i,
            anim::blend(base, sparkle, brightness)
        );
    }
}
