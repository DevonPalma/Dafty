#pragma once
// Guy Manuel Helmet Show: 48x8 matrix between two 16-pixel rings.
// Every state is a compile-time pair of a renderer and its event handler.
// Button actions and all transition rules live in this header.
// No scene arrays, heap allocation, or extra LED framebuffers.
#include <Arduino.h>
#include "LEDLayouts.h"
#include "Animations.h"
#include "StateMachine.h"

// Forward declarations are required for compile-time state bindings.
inline void ambientDream(const StateContext&);
inline void ambientSpectrum(const StateContext&);
inline void ambientSweep(const StateContext&);
inline void partyPrism(const StateContext&);
inline void partyChaos(const StateContext&);
inline void alertSignal(const StateContext&);
inline void ambientDreamEvents(Event);
inline void ambientSpectrumEvents(Event);
inline void ambientSweepEvents(Event);
inline void partyPrismEvents(Event);
inline void partyChaosEvents(Event);
inline void alertSignalEvents(Event);

using IdleA = State<ambientDream, ambientDreamEvents>;       // main.cpp starts here
using IdleB = State<ambientSpectrum, ambientSpectrumEvents>;
using Sweep = State<ambientSweep, ambientSweepEvents>;
using Party = State<partyPrism, partyPrismEvents>;
using PartyChaos = State<partyChaos, partyChaosEvents>;
using Alert = State<alertSignal, alertSignalEvents>;

// ========================================================================
// STATE RENDERERS
// Different animations run concurrently on the rings and matrix.
// main.cpp clears the framebuffer once before calling machine.render().
// ========================================================================

inline void ambientDream(const StateContext& ctx) {
  // Relaxed moving cyan wave over a dark display, with breathing rings.
  matrixWave<FaceMatrix>(Cyan, ctx.elapsedMs, 3600, 2);
  matrixRain<FaceMatrix>(Blue, ctx.elapsedMs, 210, 7, 2);
  ringBreathe<LeftRing>(Purple, ctx.elapsedMs, 2500);
  ringBreathe<RightRing>(Cyan, ctx.elapsedMs + 900, 2500);
}

inline void ambientSpectrum(const StateContext& ctx) {
  // Synthetic equalizer, 16 slim bars with smoothly changing heights.
  matrixEqualizer<FaceMatrix>(Cyan, Purple, ctx.elapsedMs, 4000, 16, 12);
  ringChase<LeftRing>(Cyan, ctx.elapsedMs, 2000, 4);
  ringChase<RightRing>(Purple, ctx.elapsedMs, 1600, 4);
}

inline void ambientSweep(const StateContext& ctx) {
  // Sharp vertical scanner with a glowing trail and opposing ring tracers.
  matrixScanner<FaceMatrix>(White, ctx.elapsedMs, 2300, 3);
  ringDualChase<LeftRing>(Cyan, Blue, ctx.elapsedMs, 2200, 3);
  ringDualChase<RightRing>(Purple, Cyan, ctx.elapsedMs, 2200, 3);
}

inline void partyPrism(const StateContext& ctx) {
  // Whole face fills with shifting color. Rings spin at different speeds.
  matrixRainbow<FaceMatrix>(ctx.elapsedMs, 2100);
  ringDualChase<LeftRing>(Cyan, Purple, ctx.elapsedMs, 900, 4);
  ringDualChase<RightRing>(Purple, Yellow, ctx.elapsedMs, 1100, 4);
}

inline void partyChaos(const StateContext& ctx) {
  // Glitch blocks over a moving wave, plus fast changing ring sparkles.
  matrixWave<FaceMatrix>(Cyan, ctx.elapsedMs, 1100, 3);
  matrixGlitch<FaceMatrix>(Purple, White, ctx.elapsedMs, 125, 11);
  ringSparkle<LeftRing>(Cyan, ctx.elapsedMs, 110, 4);
  ringSparkle<RightRing>(Purple, ctx.elapsedMs + 60, 130, 4);
}

inline void alertSignal(const StateContext& ctx) {
  // Brief red warning pattern; intentionally not a rapid strobe.
  const bool lit = (ctx.elapsedMs / 300u) % 2u == 0;
  if (lit) {
    solid<FacePanel>(Red);
    solid<BothRings>(Yellow);
  } else {
    matrixScanner<FaceMatrix>(Red, ctx.elapsedMs, 650, 1);
    ringChase<BothRings>(Red, ctx.elapsedMs, 650, 2);
  }
}

// ========================================================================
// STATE TRANSITIONS
// Timed ambient cycle: Dream -> Spectrum -> Sweep -> Dream ...
//   Single: hold this ambient state; next single advances and holds.
//   Double: party cycle Prism <-> Chaos.
//   Long: resume timed ambient cycle from Dream.
// Party: single triggers brief Alert; double holds the opposite party look.
// Alert: after 1800 ms return to timed Party Prism.
// ========================================================================

inline void ambientDreamEvents(Event e) {
  switch (e) {
    case Event::Tick:
      if (machine.finished()) machine.transition<IdleB>(4000);
      break;
    case Event::ButtonPressed:
      if (machine.timed()) machine.transition<IdleA>();
      else machine.transition<IdleB>();
      break;
    case Event::ButtonDoublePressed:
      machine.transition<Party>(3200);
      break;
    case Event::ButtonLongHeld:
      machine.transition<IdleA>(4000);
      break;
  }
}

inline void ambientSpectrumEvents(Event e) {
  switch (e) {
    case Event::Tick:
      if (machine.finished()) machine.transition<Sweep>(3800);
      break;
    case Event::ButtonPressed:
      if (machine.timed()) machine.transition<IdleB>();
      else machine.transition<Sweep>();
      break;
    case Event::ButtonDoublePressed:
      machine.transition<Party>(3200);
      break;
    case Event::ButtonLongHeld:
      machine.transition<IdleA>(4000);
      break;
  }
}

inline void ambientSweepEvents(Event e) {
  switch (e) {
    case Event::Tick:
      if (machine.finished()) machine.transition<IdleA>(4000);
      break;
    case Event::ButtonPressed:
      if (machine.timed()) machine.transition<Sweep>();
      else machine.transition<IdleA>();
      break;
    case Event::ButtonDoublePressed:
      machine.transition<Party>(3200);
      break;
    case Event::ButtonLongHeld:
      machine.transition<IdleA>(4000);
      break;
  }
}

inline void partyPrismEvents(Event e) {
  switch (e) {
    case Event::Tick:
      if (machine.finished()) machine.transition<PartyChaos>(2800);
      break;
    case Event::ButtonPressed:
      machine.transition<Alert>(1800);
      break;
    case Event::ButtonDoublePressed:
      machine.transition<PartyChaos>();
      break;
    case Event::ButtonLongHeld:
      machine.transition<IdleA>(4000);
      break;
  }
}

inline void partyChaosEvents(Event e) {
  switch (e) {
    case Event::Tick:
      if (machine.finished()) machine.transition<Party>(3200);
      break;
    case Event::ButtonPressed:
      machine.transition<Alert>(1800);
      break;
    case Event::ButtonDoublePressed:
      machine.transition<Party>();
      break;
    case Event::ButtonLongHeld:
      machine.transition<IdleA>(4000);
      break;
  }
}

inline void alertSignalEvents(Event e) {
  switch (e) {
    case Event::Tick:
      if (machine.finished()) machine.transition<Party>(3200);
      break;
    case Event::ButtonPressed:
    case Event::ButtonDoublePressed:
      machine.transition<Party>(3200);
      break;
    case Event::ButtonLongHeld:
      machine.transition<IdleA>(4000);
      break;
  }
}

inline void startHelmet() {
    machine.transition<IdleA>(4000);
}