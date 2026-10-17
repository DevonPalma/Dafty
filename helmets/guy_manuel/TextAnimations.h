#pragma once
// Draw GFX's built-in 5x7 font directly into a compile-time LED Matrix.
// No NeoMatrix instance, extra framebuffer, or blocking scroll loop.
#include <Adafruit_GFX.h>
#include <string.h>
#include "TextConfig.h"

template<class Layout>
class MatrixTextCanvas : public Adafruit_GFX {
  Color ink_;
public:
  explicit MatrixTextCanvas(Color ink)
    : Adafruit_GFX(Layout::width, Layout::height), ink_(ink) {
    static_assert(Layout::isMatrix, "MatrixText requires a matrix layout");
    setTextWrap(false);
    setTextColor(1);
  }
  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    if (!color || x < 0 || y < 0 || x >= width() || y >= height()) return;
    Layout::setXY(uint16_t(x), uint16_t(y), ink_);
  }
};

// Milliseconds per column. One text glyph is 6 columns (5 pixels + space).
template<class Layout>
inline void matrixScrollText(const char* message, Color ink, uint32_t elapsedMs,
                             uint16_t msPerColumn = 75) {
  if (!message || !msPerColumn) return;
  MatrixTextCanvas<Layout> canvas(ink);
  const uint32_t shift = elapsedMs / msPerColumn;
  const int32_t x = int32_t(Layout::width) - int32_t(shift);
  if (x < -32768L) return; // fully offscreen after long waits
  canvas.setCursor(int16_t(x), 0);
  canvas.print(message);
}

// Use same scrolling rate to calculate when the message has fully passed.
template<class Layout>
inline uint32_t matrixScrollDuration(uint8_t characters, uint16_t msPerColumn = 75) {
  return uint32_t(Layout::width + uint16_t(characters) * 6u) * msPerColumn;
}

// Center a short message (max ~8 characters on the 48px face).
// Longer text is clipped by the canvas; use *Scroll for long messages.
template<class Layout>
inline void matrixCenteredText(const char* message, Color ink) {
  if (!message) return;
  MatrixTextCanvas<Layout> canvas(ink);
  const int32_t pixelWidth = int32_t(strlen(message)) * 6;
  const int32_t startX = (int32_t(Layout::width) - pixelWidth) / 2;
  canvas.setCursor(int16_t(startX < -32768L ? -32768L : startX), 0);
  canvas.print(message);
}

// Static centered text; no position changes or blinking. 48px fits ~8 chars.
template<class Layout>
inline void matrixStaticText(const char* message, Color ink) {
  matrixCenteredText<Layout>(message, ink);
}

// Configured Guy Manuel text animation. States don't interpret draw modes.
// main.cpp clears pixels before each render, so unlit pixels stay off.
template<class Layout>
inline void textAnimation(uint32_t elapsedMs) {
  const guyText::Config& cfg = guyText::current;
  switch (cfg.draw) {
    case guyText::Draw::Scroll:
      matrixScrollText<Layout>(cfg.message, cfg.color, elapsedMs, 75);
      break;
    case guyText::Draw::Static:
      matrixStaticText<Layout>(cfg.message, cfg.color);
      break;
    case guyText::Draw::Blink:
      if ((elapsedMs / 300UL) % 2UL == 0)
        matrixCenteredText<Layout>(cfg.message, cfg.color);
      break;
  }
}

template<class Layout>
inline uint32_t textAnimationDuration() {
  switch (guyText::current.draw) {
    case guyText::Draw::Scroll:
      return matrixScrollDuration<Layout>(uint8_t(strlen(guyText::current.message)), 75);
    case guyText::Draw::Static: return 2200;
    case guyText::Draw::Blink: return 3000;
  }
  return 0;
}
