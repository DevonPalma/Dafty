#pragma once

#include <Arduino.h>

#include <math.h>

#include "AnimationTypes.h"
#include "MatrixGraphics.h"

// RENDERING IMPLEMENTATIONS (Flash-backed project)
// This file defines the settings structs and render functions referenced by
// FlashAnimations.h. Do not create arrays of Effect objects here.
// New animations need: a settings struct, a render function, an EffectKind,
// an FX_... initializer macro, and a dispatch case in FlashAnimations.h.
//
// Each render function receives p from 0 (start) to <1 (end). The scheduler
// clears the pixel buffer each frame and calls all scene effects in order.
// 1D effects follow section wiring order; they do NOT become 2D when sent
// to a matrix-shaped target. Use shape::setXY for geometry-aware effects.
// No delay(), no leds.show(), and no large automatic arrays in renderers.

// Shared color helpers used by the animation renderers.
inline float bounded(float v) {
  return v < 0 ? 0 : (v > 1 ? 1 : v);
}

inline uint8_t lerp8(uint8_t a, uint8_t b, float t) {
  return (uint8_t)(a + ((int)b - (int)a) * bounded(t));
}

inline Color blend(Color a, Color b, float t) {
  return {lerp8(a.r, b.r, t), lerp8(a.g, b.g, t), lerp8(a.b, b.b, t)};
}

inline Color scaleColor(Color c, float brightness) {
  return { (uint8_t)(c.r * bounded(brightness)),
           (uint8_t)(c.g * bounded(brightness)),
           (uint8_t)(c.b * bounded(brightness)) };
}

// 8-bit color wheel: no floating point inside a pixel loop.
inline Color wheel8(uint8_t position) {
  if (position < 85) return RGB(255 - position * 3, position * 3, 0);
  if (position < 170) { position -= 85; return RGB(0, 255 - position * 3, position * 3); }
  position -= 170;
  return RGB(position * 3, 0, 255 - position * 3);
}

// ============================================================
// FADE
// ============================================================
// Smoothly transitions from one color to another.
// Playlist: use its FX_* macro in AnimationPlaylist.h.
// Inputs: fromColor: starting color; toColor: ending color; durationMs: one fade cycle in milliseconds.
// Settings are supplied through the FX_* macro (not runtime builders).
// Example values and targets: see AnimationPlaylist.h.
struct FadeSettings {
  Color from, to;
};

inline void renderFade(const Effect& e, const LEDSection& s, float p) {
  FadeSettings cfg = effectSettings<FadeSettings>(e);
  leds.fill(s, blend(cfg.from, cfg.to, p));
}



// ============================================================
// SOLID
// ============================================================
// Displays a constant color with no movement.
// Playlist: use its FX_* macro in AnimationPlaylist.h.
// Inputs: color: displayed color; durationMs: one effect cycle in milliseconds.
// Settings are supplied through the FX_* macro (not runtime builders).
// Example values and targets: see AnimationPlaylist.h.
struct SolidSettings {
  Color color;
};

inline void renderSolid(const Effect& e, const LEDSection& s, float) {
  leds.fill(s, effectSettings<SolidSettings>(e).color);
}



// ============================================================
// PULSE
// ============================================================
// Dims a color to black, then brightens it again during each cycle.
// Playlist: use its FX_* macro in AnimationPlaylist.h.
// Inputs: color: pulse color; durationMs: one pulse cycle in milliseconds.
// Settings are supplied through the FX_* macro (not runtime builders).
// Example values and targets: see AnimationPlaylist.h.
struct PulseSettings {
  Color color;
};

inline void renderPulse(const Effect& e, const LEDSection& s, float p) {
  Color c = effectSettings<PulseSettings>(e).color;
  leds.fill(s, scaleColor(c, 1.0f - fabsf(2.0f * p - 1.0f)));
}



// ============================================================
// WIPE
// ============================================================
// Lights pixels in section order, starting at the first LED.
// Playlist: use its FX_* macro in AnimationPlaylist.h.
// Inputs: color: wipe color; durationMs: time to progress across the section.
// Settings are supplied through the FX_* macro (not runtime builders).
// Example values and targets: see AnimationPlaylist.h.
struct WipeSettings {
  Color color;
};

inline void renderWipe(const Effect& e, const LEDSection& s, float p) {
  Color c = effectSettings<WipeSettings>(e).color;
  uint16_t amount = (uint16_t)(bounded(p) * s.count);
  for (uint16_t i = 0; i < amount; ++i) leds.set(s, i, c);
}



// ============================================================
// RAINBOW
// ============================================================
// Spreads a rainbow across the section and scrolls its colors.
// Playlist: use its FX_* macro in AnimationPlaylist.h.
// Inputs: durationMs: one animation cycle in milliseconds.
// Settings are supplied through the FX_* macro (not runtime builders).
// Example values and targets: see AnimationPlaylist.h.
struct RainbowSettings {
  float rotations;

  bool reverse;
};

inline void renderRainbow(const Effect& e, const LEDSection& s, float p) {
  RainbowSettings cfg = effectSettings<RainbowSettings>(e);
  // Calculate animation displacement ONCE, not for every LED.
  int16_t movement = (int16_t)(p * cfg.rotations * 256.0f);
  if (cfg.reverse) movement = -movement;
  if (e.shape && e.shape->kind == ShapeKind::Matrix) {
    MatrixGraphics matrix(*e.shape);
    for (uint16_t x = 0; x < matrix.width(); ++x) {
      Color c = wheel8((uint8_t)((uint32_t)x * 256 / matrix.width() + movement));
      matrix.vline(x, 0, matrix.height(), c);
    }
  } else if (s.count) {
    for (uint16_t i = 0; i < s.count; ++i)
      leds.set(s, i, wheel8((uint8_t)((uint32_t)i * 256 / s.count + movement)));
  }
}


// ============================================================
// COMET
// ============================================================
// Moves two colored heads around the section with fading tails.
// Playlist: use its FX_* macro in AnimationPlaylist.h.
// Inputs: firstColor / secondColor: comet colors; durationMs: one effect cycle.
// Settings are supplied through the FX_* macro (not runtime builders).
// Example values and targets: see AnimationPlaylist.h.
struct CometSettings {
  Color first, second;
  float speed1, speed2;
  uint8_t trail;
};

inline void drawComet(const LEDSection& section, Color color, float position, uint8_t tail, bool backwards) {
  if (!section.count) return;
  int length = section.count;
  int head = (int)(position * length) % length;
  if (head < 0) head += length;
  for (uint8_t t = 0; t < tail; ++t) {
    int led = head + (backwards ? (int)t : -(int)t);

    led %= length;
    if (led < 0) led += length;
    float intensity = 1.0f - (float)t / tail;
    leds.set(section, led, scaleColor(color, intensity));
  }
}

inline void renderComet(const Effect& e, const LEDSection& s, float p) {
  CometSettings cfg = effectSettings<CometSettings>(e);

  drawComet(s, cfg.first, p * cfg.speed1, cfg.trail, false);

  drawComet(s, cfg.second, 1.0f - p * cfg.speed2, cfg.trail, true);
}





// Deterministic pseudo-random values for repeatable visual patterns.
// These animations do not read audio input.
inline uint16_t lightHash(uint16_t x) {
  x ^= x >> 7;

  x *= 40503u;

  x ^= x >> 5;
  return x;
}

// ============================================================
// EQUALIZER
// ============================================================
// Generates changing colored bars, like an audio visualizer.
// Playlist: use its FX_* macro in AnimationPlaylist.h.
// Inputs: lowColor / highColor: colors blended by bar height; durationMs: one effect cycle.
// Settings are supplied through the FX_* macro (not runtime builders).
// Example values and targets: see AnimationPlaylist.h.
struct EqualizerSettings {
  Color low, high;
  uint8_t bars;
  uint8_t beats;
};

inline void renderEqualizer(const Effect& e, const LEDSection& s, float p) {
  EqualizerSettings cfg = effectSettings<EqualizerSettings>(e);
  const uint16_t beat = (uint16_t)(bounded(p) * (cfg.beats ? cfg.beats : 1));
  if (e.shape && e.shape->kind == ShapeKind::Matrix) {
    // A 48x8 screen is a SCREEN, not a strip. Draw columns upward.
    MatrixGraphics matrix(*e.shape);
    if (!matrix.width() || !matrix.height()) return;
    uint8_t bars = cfg.bars ? cfg.bars : 1;
    if (bars > matrix.width()) bars = matrix.width();
    for (uint8_t b = 0; b < bars; ++b) {
      uint16_t startX = (uint32_t)b * matrix.width() / bars;
      uint16_t endX = (uint32_t)(b + 1) * matrix.width() / bars;
      uint8_t level = 1 + lightHash(b * 73u + beat * 101u) % matrix.height();
      // Compute color ONCE per bar. No floating point inside pixel loops.
      Color c = blend(cfg.low, cfg.high, (float)level / matrix.height());
      matrix.bar(startX, endX - startX, level, c);
    }
    return;
  }
  // Linear sections keep the old strip-style behavior.
  if (!s.count) return;
  uint8_t bars = cfg.bars ? cfg.bars : 1;
  if (bars > s.count) bars = s.count;
  for (uint8_t b = 0; b < bars; ++b) {
    uint16_t start = (uint32_t)b * s.count / bars;
    uint16_t end = (uint32_t)(b + 1) * s.count / bars;
    uint16_t level = 1 + lightHash(b * 73u + beat * 101u) % (end - start);
    Color c = blend(cfg.low, cfg.high, (float)level / (end - start));
    for (uint16_t i = 0; i < level; ++i) leds.set(s, start + i, c);
  }
}


// ============================================================
// SCANNER
// ============================================================
// Sweeps a glowing point back and forth across the section.
// Playlist: use its FX_* macro in AnimationPlaylist.h.
// Inputs: color: scanning light color; durationMs: one effect cycle.
// Settings are supplied through the FX_* macro (not runtime builders).
// Example values and targets: see AnimationPlaylist.h.
struct ScannerSettings {
  Color color;
  uint8_t trail;
  uint8_t sweeps;
};

inline void renderScanner(const Effect& e, const LEDSection& s, float p) {
  ScannerSettings cfg = effectSettings<ScannerSettings>(e);
  float phase = p * (cfg.sweeps ? cfg.sweeps : 1);
  phase -= (uint16_t)phase;
  float triangle = phase < 0.5f ? phase * 2 : 2 - phase * 2;
  if (e.shape && e.shape->kind == ShapeKind::Matrix) {
    MatrixGraphics matrix(*e.shape);
    if (!matrix.width()) return;
    int16_t head = (int16_t)(triangle * (matrix.width() - 1));
    // trail is a glow radius measured in COLUMNS, not pixel indices.
    uint8_t radius = cfg.trail ? cfg.trail : 1;
    if (radius > matrix.width()) radius = matrix.width();
    for (int16_t d = -(int16_t)radius; d <= radius; ++d) {
      uint8_t intensity = (uint32_t)(radius + 1 - abs(d)) * 255 / (radius + 1);
      Color c = { (uint8_t)((uint16_t)cfg.color.r * intensity / 255),
                  (uint8_t)((uint16_t)cfg.color.g * intensity / 255),
                  (uint8_t)((uint16_t)cfg.color.b * intensity / 255) };
      matrix.vline(head + d, 0, matrix.height(), c);
    }
    return;
  }
  if (!s.count) return;
  int16_t head = (int16_t)(triangle * (s.count - 1));
  int16_t radius = cfg.trail ? cfg.trail : 1;
  for (int16_t d = -radius; d <= radius; ++d) {
    int16_t index = head + d;
    if (index < 0 || index >= s.count) continue;
    uint8_t intensity = (uint32_t)(radius + 1 - abs(d)) * 255 / (radius + 1);
    leds.set(s, index, RGB((uint16_t)cfg.color.r * intensity / 255,
                           (uint16_t)cfg.color.g * intensity / 255,
                           (uint16_t)cfg.color.b * intensity / 255));
  }
}


// ============================================================
// GLITCH
// ============================================================
// Displays changing digital blocks with intermittent bright flashes.
// Playlist: use its FX_* macro in AnimationPlaylist.h.
// Inputs: baseColor: dim blocks; flashColor: occasional bright blocks; durationMs: one effect cycle.
// Settings are supplied through the FX_* macro (not runtime builders).
// Example values and targets: see AnimationPlaylist.h.
struct GlitchSettings {
  Color base, flash;
  uint8_t blocks;
  uint8_t changes;
};

inline void renderGlitch(const Effect& e, const LEDSection& s, float p) {
  GlitchSettings cfg = effectSettings<GlitchSettings>(e);
  uint16_t frame = (uint16_t)(bounded(p) * (cfg.changes ? cfg.changes : 1));
  if (e.shape && e.shape->kind == ShapeKind::Matrix) {
    MatrixGraphics matrix(*e.shape);
    if (!matrix.width() || !matrix.height()) return;
    // Draw a handful of scattered 2D patches, not 384/blocks long strips.
    uint8_t patches = cfg.blocks ? cfg.blocks : 1;
    if (patches > 18) patches = 18;
    for (uint8_t b = 0; b < patches; ++b) {
      uint16_t seed = lightHash(b * 313u + frame * 887u);
      uint8_t x = seed % matrix.width();
      uint8_t y = (seed >> 5) % matrix.height();
      uint8_t w = 1 + ((seed >> 9) % 9);
      uint8_t h = 1 + ((seed >> 13) % 3);
      Color c = (seed % 5 == 0) ? cfg.flash : RGB(cfg.base.r / 3, cfg.base.g / 3, cfg.base.b / 3);
      matrix.fillRect(x, y, w, h, c);
    }
    return;
  }
  if (!s.count) return;
  uint8_t blocks = cfg.blocks ? cfg.blocks : 1;
  if (blocks > s.count) blocks = s.count;
  for (uint8_t b = 0; b < blocks; ++b) {
    uint16_t seed = lightHash(b * 313u + frame * 887u);
    Color c = (seed % 5 == 0) ? cfg.flash : RGB(cfg.base.r / 3, cfg.base.g / 3, cfg.base.b / 3);
    uint16_t start = (uint32_t)b * s.count / blocks;
    uint16_t end = (uint32_t)(b + 1) * s.count / blocks;
    for (uint16_t i = start; i < end; ++i) leds.set(s, i, c);
  }
}


// ============================================================
// NEONSTEPS
// ============================================================
// Scrolls alternating color blocks separated by unlit gaps.
// Playlist: use its FX_* macro in AnimationPlaylist.h.
// Inputs: firstColor / secondColor: block colors; durationMs: one effect cycle.
// Settings are supplied through the FX_* macro (not runtime builders).
// Example values and targets: see AnimationPlaylist.h.
struct NeonStepsSettings {
  Color first, second;
  uint8_t width, cycles;
};

inline void renderNeonSteps(const Effect& e, const LEDSection& s, float p) {
  NeonStepsSettings cfg = effectSettings<NeonStepsSettings>(e);
  uint8_t width = cfg.width ? cfg.width : 1;
  int16_t shift = (int16_t)(bounded(p) * (cfg.cycles ? cfg.cycles : 1) * width * 3);
  if (e.shape && e.shape->kind == ShapeKind::Matrix) {
    MatrixGraphics matrix(*e.shape);
    // Alternate vertical COLOR | COLOR | BLANK bands, scrolling along X.
    for (uint16_t x = 0; x < matrix.width(); ++x) {
      uint8_t slot = ((x + shift) / width) % 3;
      if (slot != 2) matrix.vline(x, 0, matrix.height(), slot == 0 ? cfg.first : cfg.second);
    }
    return;
  }
  for (uint16_t i = 0; i < s.count; ++i) {
    uint8_t slot = ((uint32_t)i + shift) / width % 3;
    if (slot != 2) leds.set(s, i, slot == 0 ? cfg.first : cfg.second);
  }
}


// ============================================================
// MATRIX DOT: moves a pixel across a 2D matrix, wraps through rows.
// Playlist: use its FX_* macro in AnimationPlaylist.h.
// Requires Target::FaceMatrix (matrix coordinates, not physical wiring order).
struct MatrixDotSettings { Color color; };
inline void renderMatrixDot(const Effect& e, const LEDSection&, float p) {
  if (!e.shape || e.shape->kind != ShapeKind::Matrix) return;
  const LEDShape& sh = *e.shape;
  uint32_t n = (uint32_t)sh.width * sh.height;
  if (!n) return;
  uint32_t position = (uint32_t)(bounded(p) * n);
  if (position >= n) position = n - 1;
  Color c = effectSettings<MatrixDotSettings>(e).color;
  shape::setXY(sh, position % sh.width, position / sh.width, c);
}


// RING CHASE: spins a bright LED around a ring, with a short dim tail.
// Playlist: use its FX_* macro in AnimationPlaylist.h.
struct RingChaseSettings { Color color; };
inline void renderRingChase(const Effect& e, const LEDSection&, float p) {
  if (!e.shape || e.shape->kind != ShapeKind::Ring || !e.shape->width) return;
  const LEDShape& sh = *e.shape;
  uint16_t head = (uint16_t)(bounded(p) * sh.width) % sh.width;
  Color c = effectSettings<RingChaseSettings>(e).color;
  for (uint8_t i = 0; i < 4; ++i) {
    uint16_t pos = (head + sh.width - i % sh.width) % sh.width;
    shape::set(sh, pos, scaleColor(c, 1.0f - i * 0.23f));
  }
}


// ============================================================================
// MATRIX TEST: Four phases designed to diagnose matrix coordinate mapping.
// Usage in Flash playlist: FX_MATRIXTEST(Target::FaceMatrix, 16000)
// 0-25%:  four corners (TL red, TR green, BL blue, BR white).
// 25-50%: a full yellow horizontal row moves TOP to BOTTOM.
// 50-75%: a full cyan vertical column moves LEFT to RIGHT.
// 75-100%: eight consecutive RAW, physically wired LEDs light magenta.
//
// The physical wiring phase shows eight consecutive LEDs, which may be a
// partial ROW for the currently configured row-major matrix. If the
// corners/rows/columns do not match expected positions,
// check MatrixOrder, zigzag, reverse, and matrix orientation.
// No additional pixel buffer, timers, or SRAM state are needed.
// ============================================================================
inline void renderMatrixTest(const Effect& e, const LEDSection& s, float progress) {
  if (!e.shape || e.shape->kind != ShapeKind::Matrix) return;
  const LEDShape& m = *e.shape;
  if (m.width == 0 || m.height == 0) return;
  float p = bounded(progress);

  if (p < 0.25f) {
    shape::setXY(m, 0, 0, Red);                       // top-left
    shape::setXY(m, m.width - 1, 0, Green);           // top-right
    shape::setXY(m, 0, m.height - 1, Blue);           // bottom-left
    shape::setXY(m, m.width - 1, m.height - 1, White);// bottom-right
  } else if (p < 0.50f) {
    float phase = (p - 0.25f) * 4.0f;
    uint16_t y = (uint16_t)(phase * m.height);
    if (y >= m.height) y = m.height - 1;
    for (uint16_t x = 0; x < m.width; ++x)
      shape::setXY(m, x, y, Yellow);
  } else if (p < 0.75f) {
    float phase = (p - 0.50f) * 4.0f;
    uint16_t x = (uint16_t)(phase * m.width);
    if (x >= m.width) x = m.width - 1;
    for (uint16_t y = 0; y < m.height; ++y)
      shape::setXY(m, x, y, Cyan);
  } else {
    // Bypass matrix mapping: illuminate a group of raw physical indices.
    // This phase intentionally bypasses the matrix X/Y mapping.
    uint16_t groups = (s.count + 7) / 8;
    uint16_t group = (uint16_t)((p - 0.75f) * 4.0f * groups);
    if (group >= groups) group = groups - 1;
    uint16_t begin = group * 8;
    for (uint16_t i = begin; i < begin + 8 && i < s.count; ++i)
      leds.set(s, i, Purple);
  }
}
