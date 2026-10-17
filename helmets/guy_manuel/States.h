#pragma once
// Guy Manuel Helmet Show: a single 48x8 matrix.
// Every state is a compile-time pair of a renderer and its event handler.
// Button actions and all transition rules live in this header.
// No scene arrays, heap allocation, or extra LED framebuffers.
#include <Arduino.h>
#include "LEDLayouts.h"
#include "Animations.h"
#include "StateMachine.h"
#ifdef DAFTY_ENABLE_BLE
#include "BluetoothText.h"
#include "Commands.h"
#include "SpecialAnimations.h"
#include "TextAnimations.h"
#endif

// Forward declarations are required for compile-time state bindings.
inline void ambientDream(const StateContext&);
inline void ambientSpectrum(const StateContext&);
inline void ambientSweep(const StateContext&);
inline void partyPrism(const StateContext&);
inline void partyChaos(const StateContext&);
inline void alertSignal(const StateContext&);
#ifdef DAFTY_ENABLE_BLE
inline void messageRender(const StateContext&);
inline void messageEvents(Event);
inline void specialRender(const StateContext&);
inline void specialEvents(Event);
#endif
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
#ifdef DAFTY_ENABLE_BLE
using Message = State<messageRender, messageEvents>;
using SpecialSequence = State<specialRender, specialEvents>;
#endif

// ========================================================================
// STATE RENDERERS
// All animations draw directly to the face matrix.
// main.cpp clears the framebuffer once before calling machine.render().
// ========================================================================

inline void ambientDream(const StateContext& ctx) {
  // Relaxed moving cyan wave and digital rain.
  matrixWave<FaceMatrix>(Cyan, ctx.elapsedMs, 3600, 2);
  matrixRain<FaceMatrix>(Blue, ctx.elapsedMs, 210, 7, 2);

}

inline void ambientSpectrum(const StateContext& ctx) {
  // Synthetic equalizer, 16 slim bars with smoothly changing heights.
  matrixEqualizer<FaceMatrix>(Cyan, Purple, ctx.elapsedMs, 4000, 16, 12);

}

inline void ambientSweep(const StateContext& ctx) {
  // Sharp vertical scanner with a glowing trail.
  matrixScanner<FaceMatrix>(White, ctx.elapsedMs, 2300, 3);

}

inline void partyPrism(const StateContext& ctx) {
  // Whole face fills with shifting color.
  matrixRainbow<FaceMatrix>(ctx.elapsedMs, 2100);

}

inline void partyChaos(const StateContext& ctx) {
  // Glitch blocks over a moving wave.
  matrixWave<FaceMatrix>(Cyan, ctx.elapsedMs, 1100, 3);
  matrixGlitch<FaceMatrix>(Purple, White, ctx.elapsedMs, 125, 11);

}

inline void alertSignal(const StateContext& ctx) {
  // Brief red warning pattern; intentionally not a rapid strobe.
  const bool lit = (ctx.elapsedMs / 300u) % 2u == 0;
  if (lit) {
    solid<FacePanel>(Red);
  } else {
    matrixScanner<FaceMatrix>(Red, ctx.elapsedMs, 650, 1);

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
#ifdef DAFTY_ENABLE_BLE
// The text animation owns all display-mode rendering and timing.
// States only decide what happens when that animation is finished.
inline void commandDestination() {
  using namespace guyCommands;
  switch (current.next) {
    case Destination::Dream:    machine.transition<IdleA>(4000); break;
    case Destination::Spectrum: machine.transition<IdleB>(4000); break;
    case Destination::Sweep:    machine.transition<Sweep>(3800); break;
    case Destination::Party:    machine.transition<Party>(3200); break;
    case Destination::Chaos:    machine.transition<PartyChaos>(2800); break;
    case Destination::Alert:    machine.transition<Alert>(1800); break;
  }
}

inline void messageRender(const StateContext& ctx) {
  textAnimation<FaceMatrix>(ctx.elapsedMs);
}

inline void messageEvents(Event e) {
  switch (e) {
    case Event::Tick:
      if (machine.elapsed() >= textAnimationDuration<FaceMatrix>()) commandDestination();
      break;
    case Event::ButtonPressed:
    case Event::ButtonLongHeld:
      machine.transition<IdleA>(4000);
      break;
    case Event::ButtonDoublePressed:
      machine.transition<Party>(3200);
      break;
  }
}

inline void specialRender(const StateContext& ctx) {
  switch (guyCommands::current.special) {
    case guyCommands::Special::WorkIt: workIt<FaceMatrix>(ctx.elapsedMs); break;
    case guyCommands::Special::HeartEyes: heartEyes<FaceMatrix>(ctx.elapsedMs); break;
    default: break;
  }
}
inline void specialEvents(Event e) {
  switch (e) {
    case Event::Tick:
      if (machine.finished()) machine.transition<IdleA>(4000);
      break;
    case Event::ButtonPressed:
    case Event::ButtonLongHeld:
      machine.transition<IdleA>(4000);
      break;
    case Event::ButtonDoublePressed:
      machine.transition<Party>(3200);
      break;
  }
}

// Called from main.cpp only in Guy Manuel's BLE build.
inline void helmetOnTextMessage() {
  guyCommands::parse(bluetoothText::message());
  if (guyCommands::current.special == guyCommands::Special::WorkIt)
    machine.transition<SpecialSequence>(7200);
  else if (guyCommands::current.special == guyCommands::Special::HeartEyes)
    machine.transition<SpecialSequence>(6000);
  else if (guyText::current.message[0])
    machine.transition<Message>();
}
#endif
