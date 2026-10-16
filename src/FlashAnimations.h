#pragma once
// FLASH EFFECT DEFINITIONS AND DISPATCH
// 1) FX_* macros generate constant FlashEffect initializers.
// 2) AnimationPlaylist.h stores these records in PROGMEM.
// 3) renderFlashEffect() reads ONE descriptor from Flash, resolves its target,
//    and creates a short-lived stack adapter for the renderer in Animations.h.
// Never dereference Flash-backed LEDSection / LEDShape definitions directly
// on AVR; resolveTarget() makes RAM snapshots. No LED buffers are copied.


#include "ShapeDefinitions.h"
#include "Animations.h"  // Reuse all existing rendering algorithms
#include "FlashStorage.h"

// Constant, Flash-storable animation description.
// No builder constructors and no dynamic initialization.
enum class EffectKind : uint8_t {
  Solid, Fade, Pulse, Wipe, Rainbow, Comet, Equalizer,
  Scanner, Glitch, NeonSteps, MatrixDot, RingChase, MatrixTest
};
enum class Target : uint8_t { All, LeftRing, FacePanel, RightRing, BothRings,
  LeftRingShape, FaceMatrix, RightRingShape };
struct FlashParams {
  Color a, b;
  float f1, f2;
  uint8_t n1, n2;
  bool flag;
};
struct FlashEffect {
  EffectKind kind;
  Target target;
  unsigned long duration;
  FlashParams parameters;
};

// Every macro is a constant aggregate initializer that works with PROGMEM.
// n values are deliberately explicit so the memory model is obvious.
#define FX_SOLID(t,c,ms)          {EffectKind::Solid, t, ms, {c, Black, 0,0,0,0,false}}
#define FX_FADE(t,a,b,ms)         {EffectKind::Fade, t, ms, {a,b,0,0,0,0,false}}
#define FX_PULSE(t,c,ms)          {EffectKind::Pulse,t,ms,{c,Black,0,0,0,0,false}}
#define FX_WIPE(t,c,ms)           {EffectKind::Wipe,t,ms,{c,Black,0,0,0,0,false}}
#define FX_RAINBOW(t,ms,speed,rev) {EffectKind::Rainbow,t,ms,{Black,Black,speed,0,0,0,rev}}
#define FX_COMET(t,a,b,ms,s1,s2,trail) {EffectKind::Comet,t,ms,{a,b,s1,s2,trail,0,false}}
#define FX_EQUALIZER(t,a,b,ms,bars,beats) {EffectKind::Equalizer,t,ms,{a,b,0,0,bars,beats,false}}
#define FX_SCANNER(t,c,ms,trail,sweeps) {EffectKind::Scanner,t,ms,{c,Black,0,0,trail,sweeps,false}}
#define FX_GLITCH(t,a,b,ms,blocks,changes) {EffectKind::Glitch,t,ms,{a,b,0,0,blocks,changes,false}}
#define FX_NEONSTEPS(t,a,b,ms,width,cycles) {EffectKind::NeonSteps,t,ms,{a,b,0,0,width,cycles,false}}
#define FX_MATRIXDOT(t,c,ms)     {EffectKind::MatrixDot,t,ms,{c,Black,0,0,0,0,false}}
#define FX_RINGCHASE(t,c,ms)     {EffectKind::RingChase,t,ms,{c,Black,0,0,0,0,false}}
#define FX_MATRIXTEST(t,ms)     {EffectKind::MatrixTest,t,ms,{Black,Black,0,0,0,0,false}}

// Resolve a Flash-resident section/shape into a small STACK-owned snapshot.
// The renderer only uses it synchronously, so the pointers never escape.
inline void resolveTarget(Target target, LEDSection& section, LEDShape& shape, bool& hasShape) {
  const LEDSection* sectionInFlash = &All;
  const LEDShape* shapeInFlash = nullptr;
  switch (target) {
    case Target::LeftRing: sectionInFlash = &LeftRing; break;
    case Target::FacePanel: sectionInFlash = &FacePanel; break;
    case Target::RightRing: sectionInFlash = &RightRing; break;
    case Target::BothRings: sectionInFlash = &BothRings; break;
    case Target::LeftRingShape: shapeInFlash = &LeftRingShape; sectionInFlash = &LeftRing; break;
    case Target::FaceMatrix: shapeInFlash = &FaceMatrix; sectionInFlash = &FacePanel; break;
    case Target::RightRingShape: shapeInFlash = &RightRingShape; sectionInFlash = &RightRing; break;
    default: break;
  }
  section = flashRead(sectionInFlash);
  if (section.spanCount == 1) section.fastSpan = flashRead(section.spans);
  hasShape = shapeInFlash != nullptr;
  if (hasShape) {
    shape = flashRead(shapeInFlash);
    shape.section = &section; // never point directly into AVR Flash!
  }
}

template<typename T>
inline void applySettings(Effect& active, const T& cfg, EffectRender renderer) {
  static_assert(sizeof(T) <= EFFECT_CONFIG_BYTES, "Effect settings exceed temporary storage");
  memcpy(active.config, &cfg, sizeof(T));
  active.render = renderer;
}

// Build one small, temporary Effect from a Flash descriptor; render it;
// discard it. No playlist/scene/effect arrays occupy permanent SRAM.
inline void renderFlashEffect(const FlashEffect& def, float progress) {
  LEDSection section;
  LEDShape shape = {};
  bool hasShape = false;
  resolveTarget(def.target, section, shape, hasShape);
  Effect active = {};
  active.shape = hasShape ? &shape : nullptr;
  active.section = &section;
  const FlashParams& p = def.parameters;
  switch (def.kind) {
    case EffectKind::Solid: applySettings(active, SolidSettings{p.a}, renderSolid); break;
    case EffectKind::Fade: applySettings(active, FadeSettings{p.a,p.b}, renderFade); break;
    case EffectKind::Pulse: applySettings(active, PulseSettings{p.a}, renderPulse); break;
    case EffectKind::Wipe: applySettings(active, WipeSettings{p.a}, renderWipe); break;
    case EffectKind::Rainbow: applySettings(active, RainbowSettings{p.f1,p.flag}, renderRainbow); break;
    case EffectKind::Comet: applySettings(active, CometSettings{p.a,p.b,p.f1,p.f2,p.n1}, renderComet); break;
    case EffectKind::Equalizer: applySettings(active, EqualizerSettings{p.a,p.b,p.n1,p.n2}, renderEqualizer); break;
    case EffectKind::Scanner: applySettings(active, ScannerSettings{p.a,p.n1,p.n2}, renderScanner); break;
    case EffectKind::Glitch: applySettings(active, GlitchSettings{p.a,p.b,p.n1,p.n2}, renderGlitch); break;
    case EffectKind::NeonSteps: applySettings(active, NeonStepsSettings{p.a,p.b,p.n1,p.n2}, renderNeonSteps); break;
    case EffectKind::MatrixDot: applySettings(active, MatrixDotSettings{p.a}, renderMatrixDot); break;
    case EffectKind::RingChase: applySettings(active, RingChaseSettings{p.a}, renderRingChase); break;
    case EffectKind::MatrixTest: active.render = renderMatrixTest; break;
  }
  if (active.render) active.render(active, section, progress);
}
