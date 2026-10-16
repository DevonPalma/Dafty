#pragma once
// Compact Flash-only effect descriptions. All scene arrays remain PROGMEM.
#include "LEDLayouts.h"
#include "FlashStorage.h"

enum class EffectKind : uint8_t
{
  Solid,
  Fade,
  Pulse,
  Wipe,
  Rainbow,
  Comet,
  Equalizer,
  Scanner,
  Glitch,
  NeonSteps,
  MatrixDot,
  RingChase,
  MatrixTest
};
enum class Target : uint8_t
{
  All,
  LeftRing,
  FacePanel,
  RightRing,
  BothRings,
  LeftRingShape,
  FaceMatrix,
  RightRingShape
};
struct FlashParams
{
  Color a, b;
  float f1, f2;
  uint8_t n1, n2;
  bool flag;
};
struct FlashEffect
{
  EffectKind kind;
  Target target;
  uint32_t duration;
  FlashParams parameters;
};

#define FX_SOLID(t, c, ms)                                    \
  {                                                           \
    EffectKind::Solid, t, ms, { c, Black, 0, 0, 0, 0, false } \
  }
#define FX_FADE(t, a, b, ms)                             \
  {                                                      \
    EffectKind::Fade, t, ms, { a, b, 0, 0, 0, 0, false } \
  }
#define FX_PULSE(t, c, ms)                                    \
  {                                                           \
    EffectKind::Pulse, t, ms, { c, Black, 0, 0, 0, 0, false } \
  }
#define FX_WIPE(t, c, ms)                                    \
  {                                                          \
    EffectKind::Wipe, t, ms, { c, Black, 0, 0, 0, 0, false } \
  }
#define FX_RAINBOW(t, ms, speed, rev)                                 \
  {                                                                   \
    EffectKind::Rainbow, t, ms, { Black, Black, speed, 0, 0, 0, rev } \
  }
#define FX_COMET(t, a, b, ms, s1, s2, trail)                    \
  {                                                             \
    EffectKind::Comet, t, ms, { a, b, s1, s2, trail, 0, false } \
  }
#define FX_EQUALIZER(t, a, b, ms, bars, beats)                       \
  {                                                                  \
    EffectKind::Equalizer, t, ms, { a, b, 0, 0, bars, beats, false } \
  }
#define FX_SCANNER(t, c, ms, trail, sweeps)                              \
  {                                                                      \
    EffectKind::Scanner, t, ms, { c, Black, 0, 0, trail, sweeps, false } \
  }
#define FX_GLITCH(t, a, b, ms, blocks, changes)                       \
  {                                                                   \
    EffectKind::Glitch, t, ms, { a, b, 0, 0, blocks, changes, false } \
  }
#define FX_NEONSTEPS(t, a, b, ms, width, cycles)                       \
  {                                                                    \
    EffectKind::NeonSteps, t, ms, { a, b, 0, 0, width, cycles, false } \
  }
#define FX_MATRIXDOT(t, c, ms)                                    \
  {                                                               \
    EffectKind::MatrixDot, t, ms, { c, Black, 0, 0, 0, 0, false } \
  }
#define FX_RINGCHASE(t, c, ms)                                    \
  {                                                               \
    EffectKind::RingChase, t, ms, { c, Black, 0, 0, 0, 0, false } \
  }
#define FX_MATRIXTEST(t, ms)                                           \
  {                                                                    \
    EffectKind::MatrixTest, t, ms, { Black, Black, 0, 0, 0, 0, false } \
  }
