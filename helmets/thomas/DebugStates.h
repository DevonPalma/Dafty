#pragma once
// Standalone helmet diagnostic states. Include INSTEAD of States.h.
// Requires LEDLayouts.h, LEDGeometry.h, animations/Common.h, StateMachine.h.
#include <Arduino.h>
#include "LEDLayouts.h"
#include "Animations.h"
#include "StateMachine.h"

namespace helmetDebug
{

  template <class Layout>
  inline void cornerGradient(uint32_t elapsedMs)
  {
    static_assert(Layout::isMatrix, "cornerGradient needs a matrix");

    // 2 seconds ON, 2 seconds OFF.
    const bool visible = (elapsedMs / 2000UL) % 2 == 0;

    const uint16_t maxX = Layout::width > 1 ? Layout::width - 1 : 1;
    const uint16_t maxY = Layout::height > 1 ? Layout::height - 1 : 1;

    for (uint16_t y = 0; y < Layout::height; ++y)
    {
      const uint16_t fy = uint32_t(y) * 255UL / maxY;

      for (uint16_t x = 0; x < Layout::width; ++x)
      {

        // Keep all four corners permanently illuminated.
        const bool isCorner =
            (x == 0 || x == maxX) &&
            (y == 0 || y == maxY);

        if (!visible && !isCorner)
        {
          Layout::setXY(x, y, Color{0, 0, 0});
          continue;
        }

        const uint16_t fx = uint32_t(x) * 255UL / maxX;

        const uint8_t topR = 255 - fx, topG = fx;
        const uint8_t bottomR = fx, bottomG = fx;

        const uint8_t r =
            (uint32_t(topR) * (255 - fy) +
             uint32_t(bottomR) * fy + 127) /
            255;

        const uint8_t g =
            (uint32_t(topG) * (255 - fy) +
             uint32_t(bottomG) * fy + 127) /
            255;

        const uint8_t b = fy;

        Layout::setXY(x, y, Color{r, g, b});
      }
    }
  }

  template <class Layout>
  inline void scanRow(uint32_t elapsedMs)
  {
    static_assert(Layout::isMatrix, "scanRow needs a matrix");
    const uint16_t y = (elapsedMs / 600UL) % Layout::height;
    for (uint16_t x = 0; x < Layout::width; ++x)
      Layout::setXY(x, y, Color{255, 255, 255});
  }

  template <class Layout>
  inline void scanColumn(uint32_t elapsedMs)
  {
    static_assert(Layout::isMatrix, "scanColumn needs a matrix");
    const uint16_t x = (elapsedMs / 300UL) % Layout::width;
    for (uint16_t y = 0; y < Layout::height; ++y)
      Layout::setXY(x, y, Color{255, 255, 255});
  }

  inline void rainbow(const StateContext &ctx)
  {
    (void)ctx;
    cornerGradient<LeftVisor>(ctx.elapsedMs);
    cornerGradient<RightVisor>(ctx.elapsedMs);
  }
  inline void rows(const StateContext &ctx)
  {
    scanRow<LeftVisor>(ctx.elapsedMs);
    scanRow<RightVisor>(ctx.elapsedMs);
  }
  inline void columns(const StateContext &ctx)
  {
    scanColumn<LeftVisor>(ctx.elapsedMs);
    scanColumn<RightVisor>(ctx.elapsedMs);
  }
  inline void pixelsOneByOne(const StateContext &ctx)
  {
    const uint16_t led = (ctx.elapsedMs / 150UL) % LED_COUNT;
    All::set(led, Color{255, 255, 255});
  }

  inline void rainbowEvents(Event);
  inline void rowEvents(Event);
  inline void columnEvents(Event);
  inline void pixelEvents(Event);
  using Rainbow = State<rainbow, rainbowEvents>;
  using Rows = State<rows, rowEvents>;
  using Columns = State<columns, columnEvents>;
  using Pixels = State<pixelsOneByOne, pixelEvents>;

  inline void rainbowEvents(Event e)
  {
    if (e == Event::ButtonPressed)
      machine.transition<Rows>();
    else if (e == Event::ButtonDoublePressed)
      machine.transition<Pixels>();
    else if (e == Event::ButtonLongHeld)
      machine.transition<Rainbow>();
  }
  inline void rowEvents(Event e)
  {
    if (e == Event::ButtonPressed)
      machine.transition<Columns>();
    else if (e == Event::ButtonDoublePressed)
      machine.transition<Rainbow>();
    else if (e == Event::ButtonLongHeld)
      machine.transition<Rainbow>();
  }
  inline void columnEvents(Event e)
  {
    if (e == Event::ButtonPressed)
      machine.transition<Pixels>();
    else if (e == Event::ButtonDoublePressed)
      machine.transition<Rows>();
    else if (e == Event::ButtonLongHeld)
      machine.transition<Rainbow>();
  }
  inline void pixelEvents(Event e)
  {
    if (e == Event::ButtonPressed)
      machine.transition<Rainbow>();
    else if (e == Event::ButtonDoublePressed)
      machine.transition<Columns>();
    else if (e == Event::ButtonLongHeld)
      machine.transition<Rainbow>();
  }
} // namespace helmetDebug

inline void startHelmet() { machine.transition<helmetDebug::Rainbow>(); }
