#pragma once
// Thomas Helmet Show: Two 20-LED visors, two 10-LED ears, and a 20-LED rear.
// The state is a renderer + event handler bound together at compile time.
// No heap allocations, no animation registries, and no separate framebuffers.
#include <Arduino.h>
#include "LEDLayouts.h"
#include "Animations.h"
#include "StateMachine.h"

// Declare renderers and handlers before binding the compile-time state types.
inline void ambientGlow(const StateContext&);
inline void ambientChase(const StateContext&);
inline void ambientCharge(const StateContext&);
inline void partyFlow(const StateContext&);
inline void partyRush(const StateContext&);
inline void alertFlash(const StateContext&);
inline void ambientGlowEvents(Event);
inline void ambientChaseEvents(Event);
inline void ambientChargeEvents(Event);
inline void partyFlowEvents(Event);
inline void partyRushEvents(Event);
inline void alertFlashEvents(Event);

using IdleA = State<ambientGlow, ambientGlowEvents>;   // Keeps main.cpp compatible.
using IdleB = State<ambientChase, ambientChaseEvents>;
using Charge = State<ambientCharge, ambientChargeEvents>;
using Party = State<partyFlow, partyFlowEvents>;
using PartyRush = State<partyRush, partyRushEvents>;
using Alert = State<alertFlash, alertFlashEvents>;

// ---------------------------------------------------------------------------
// RENDERERS: combine existing reusable strip effects.
// Individual strips are addressed by compile-time LEDLayouts aliases.
// ---------------------------------------------------------------------------

inline void ambientGlow(const StateContext& ctx) {
  // Calm, asymmetrical color breathing, with a soft rear trail.
  pulse<LeftVisor>(Cyan, ctx.elapsedMs, 2800);
  pulse<RightVisor>(Purple, ctx.elapsedMs + 950, 2800);
  pulse<BothEars>(Blue, ctx.elapsedMs, 2200);
  comet<Rear>(Cyan, ctx.elapsedMs, 3400, 6);
}

inline void ambientChase(const StateContext& ctx) {
  // Two independent tracer beams race across the visor strips.
  comet<LeftVisor>(Cyan, ctx.elapsedMs, 1350, 6);
  comet<RightVisor>(Purple, ctx.elapsedMs, 1600, 6);
  pulse<LeftEar>(Purple, ctx.elapsedMs, 1200);
  pulse<RightEar>(Cyan, ctx.elapsedMs + 600, 1200);
  comet<Rear>(White, ctx.elapsedMs, 1900, 7);
}

inline void ambientCharge(const StateContext& ctx) {
  // Visors fill up. A steady glow remains behind the charging accents.
  wipe<LeftVisor>(Cyan, ctx.elapsedMs, 2400);
  wipe<RightVisor>(Purple, ctx.elapsedMs, 2400);
  pulse<BothEars>(Yellow, ctx.elapsedMs, 800);
  pulse<Rear>(Blue, ctx.elapsedMs, 2400);
}

inline void partyFlow(const StateContext& ctx) {
  rainbow<BothVisors>(ctx.elapsedMs, 1800);
  comet<LeftEar>(Cyan, ctx.elapsedMs, 600, 4);
  comet<RightEar>(Purple, ctx.elapsedMs, 750, 4);
  rainbow<Rear>(ctx.elapsedMs, 2100);
}

inline void partyRush(const StateContext& ctx) {
  comet<LeftVisor>(White, ctx.elapsedMs, 430, 8);
  comet<RightVisor>(Cyan, ctx.elapsedMs, 530, 8);
  rainbow<BothEars>(ctx.elapsedMs, 800);
  rainbow<Rear>(ctx.elapsedMs, 650);
}

inline void alertFlash(const StateContext& ctx) {
  // Deterministic urgent on/off strobe, brief enough not to dominate the show.
  const bool on = ((ctx.elapsedMs / 130) % 2) == 0;
  if (on) {
    solid<BothVisors>(Red);
    solid<BothEars>(Yellow);
    solid<Rear>(Red);
  }
}

// ---------------------------------------------------------------------------
// TRANSITIONS: every state owns its own button and timed behavior.
//
// Ambient (timed): Glow -> Chase -> Charge -> Glow ...
//   Single: hold current ambient look; next single advances to next, held.
//   Double: start Party (Flow <-> Rush, timed).
//   Long: resume the automatic ambient loop from Glow.
// Party:
//   Single: short Alert, then return to Party.
//   Double: hold/switch to the other party look.
//   Long: resume ambient.
// ---------------------------------------------------------------------------

inline void ambientGlowEvents(Event e) {
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

inline void ambientChaseEvents(Event e) {
  switch (e) {
    case Event::Tick:
      if (machine.finished()) machine.transition<Charge>(3200);
      break;
    case Event::ButtonPressed:
      if (machine.timed()) machine.transition<IdleB>();
      else machine.transition<Charge>();
      break;
    case Event::ButtonDoublePressed:
      machine.transition<Party>(3200);
      break;
    case Event::ButtonLongHeld:
      machine.transition<IdleA>(4000);
      break;
  }
}

inline void ambientChargeEvents(Event e) {
  switch (e) {
    case Event::Tick:
      if (machine.finished()) machine.transition<IdleA>(4000);
      break;
    case Event::ButtonPressed:
      if (machine.timed()) machine.transition<Charge>();
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

inline void partyFlowEvents(Event e) {
  switch (e) {
    case Event::Tick:
      if (machine.finished()) machine.transition<PartyRush>(2600);
      break;
    case Event::ButtonPressed:
      machine.transition<Alert>(1800);
      break;
    case Event::ButtonDoublePressed:
      machine.transition<PartyRush>();
      break;
    case Event::ButtonLongHeld:
      machine.transition<IdleA>(4000);
      break;
  }
}

inline void partyRushEvents(Event e) {
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

inline void alertFlashEvents(Event e) {
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