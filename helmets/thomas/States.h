#pragma once
// Thomas: Classic visor + three independent personalities.
// Single press: next effect in current personality.
// Double press: Calm -> Crazy -> Angry -> Calm (starts at effect 1).
// Long hold: Classic visor from anywhere.
#include <Arduino.h>
#include "LEDLayouts.h"
#include "Animations.h"
#include "StateMachine.h"
#include "animations/VisorAnimations.h"

// Renderers
inline void classicIdle(const StateContext&);
inline void calmBreathe(const StateContext&);
inline void calmDrift(const StateContext&);
inline void calmStarlight(const StateContext&);
inline void crazyRainbow(const StateContext&);
inline void crazyChase(const StateContext&);
inline void crazyRush(const StateContext&);
inline void angryEmbers(const StateContext&);
inline void angryWarning(const StateContext&);
inline void angryRage(const StateContext&);

// Each personality shares one transition handler, but all behavior stays in Thomas.
inline void classicEvents(Event);
inline void calmEvents(Event);
inline void crazyEvents(Event);
inline void angryEvents(Event);

using IdleA = State<classicIdle, classicEvents>;
using Calm1 = State<calmBreathe, calmEvents>;
using Calm2 = State<calmDrift, calmEvents>;
using Calm3 = State<calmStarlight, calmEvents>;
using Crazy1 = State<crazyRainbow, crazyEvents>;
using Crazy2 = State<crazyChase, crazyEvents>;
using Crazy3 = State<crazyRush, crazyEvents>;
using Angry1 = State<angryEmbers, angryEvents>;
using Angry2 = State<angryWarning, angryEvents>;
using Angry3 = State<angryRage, angryEvents>;

// CLASSIC: unchanged colored panels, with independent blue sparkles below.
inline void classicIdle(const StateContext& ctx) {
  classicVisorPanel<LeftUpper>();
  classicVisorPanel<RightUpper>();
  twinkle<LeftLower>(Black, Cyan, ctx.elapsedMs, 1500);
  twinkle<RightLower>(Black, Cyan, ctx.elapsedMs + 400, 1500);
}

// CALM: slower and gentler; three different visual signatures.
inline void calmBreathe(const StateContext& ctx) {
  pulse<LeftVisor>(Cyan, ctx.elapsedMs, 3200);
  pulse<RightVisor>(Purple, ctx.elapsedMs + 1050, 3200);
  pulse<BothEars>(Blue, ctx.elapsedMs, 2600);
}
inline void calmDrift(const StateContext& ctx) {
  fade<LeftVisor>(Blue, Cyan, ctx.elapsedMs, 4400);
  fade<RightVisor>(Purple, Blue, ctx.elapsedMs + 2100, 4400);
  pulse<BothEars>(Cyan, ctx.elapsedMs, 3600);
}
inline void calmStarlight(const StateContext& ctx) {
  solid<BothVisors>(RGB(0, 0, 10));
  twinkle<LeftVisor>(RGB(0, 0, 10), Cyan, ctx.elapsedMs, 2700);
  twinkle<RightVisor>(RGB(0, 0, 10), Purple, ctx.elapsedMs + 870, 2700);
  pulse<BothEars>(Blue, ctx.elapsedMs, 3600);
}

// CRAZY: colorful movement, with three distinct effects.
inline void crazyRainbow(const StateContext& ctx) {
  rainbow<LeftVisor>(ctx.elapsedMs, 1300);
  rainbow<RightVisor>(ctx.elapsedMs + 650, 1300);
  pulse<BothEars>(White, ctx.elapsedMs, 850);
}
inline void crazyChase(const StateContext& ctx) {
  comet<LeftVisor>(Cyan, ctx.elapsedMs, 620, 11);
  comet<RightVisor>(Purple, ctx.elapsedMs + 310, 620, 11);
  rainbow<BothEars>(ctx.elapsedMs, 850);
}
inline void crazyRush(const StateContext& ctx) {
  wipe<LeftVisor>(Yellow, ctx.elapsedMs, 680);
  wipe<RightVisor>(RGB(255, 20, 110), ctx.elapsedMs + 340, 680);
  comet<LeftEar>(Cyan, ctx.elapsedMs, 430, 2);
  comet<RightEar>(Green, ctx.elapsedMs + 160, 430, 2);
}

// ANGRY: hot colors, heavier motion, ominous slow warning pulse.
inline void angryEmbers(const StateContext& ctx) {
  pulse<LeftVisor>(Red, ctx.elapsedMs, 1700);
  pulse<RightVisor>(RGB(255, 65, 0), ctx.elapsedMs + 520, 1700);
  solid<BothEars>(RGB(60, 0, 0));
}
inline void angryWarning(const StateContext& ctx) {
  // Alternating warning lights; deliberate 1.2s rhythm rather than fast strobing.
  if ((ctx.elapsedMs / 600UL) % 2 == 0) {
    solid<LeftVisor>(Red);
    solid<RightVisor>(RGB(45, 0, 0));
  } else {
    solid<LeftVisor>(RGB(45, 0, 0));
    solid<RightVisor>(Red);
  }
  pulse<BothEars>(Yellow, ctx.elapsedMs, 1200);
}
inline void angryRage(const StateContext& ctx) {
  comet<LeftVisor>(Red, ctx.elapsedMs, 540, 15);
  comet<RightVisor>(RGB(255, 105, 0), ctx.elapsedMs + 270, 540, 15);
  pulse<BothEars>(Red, ctx.elapsedMs, 800);
}

inline void classicEvents(Event event) {
  switch (event) {
    case Event::ButtonPressed:
    case Event::ButtonDoublePressed: machine.transition<Calm1>(); break;
    case Event::ButtonLongHeld: machine.transition<IdleA>(); break;
    case Event::Tick: break;
  }
}
inline void calmEvents(Event event) {
  switch (event) {
    case Event::ButtonPressed:
      if (machine.is<Calm1>()) machine.transition<Calm2>();
      else if (machine.is<Calm2>()) machine.transition<Calm3>();
      else machine.transition<Calm1>();
      break;
    case Event::ButtonDoublePressed: machine.transition<Crazy1>(); break;
    case Event::ButtonLongHeld: machine.transition<IdleA>(); break;
    case Event::Tick: break;
  }
}
inline void crazyEvents(Event event) {
  switch (event) {
    case Event::ButtonPressed:
      if (machine.is<Crazy1>()) machine.transition<Crazy2>();
      else if (machine.is<Crazy2>()) machine.transition<Crazy3>();
      else machine.transition<Crazy1>();
      break;
    case Event::ButtonDoublePressed: machine.transition<Angry1>(); break;
    case Event::ButtonLongHeld: machine.transition<IdleA>(); break;
    case Event::Tick: break;
  }
}
inline void angryEvents(Event event) {
  switch (event) {
    case Event::ButtonPressed:
      if (machine.is<Angry1>()) machine.transition<Angry2>();
      else if (machine.is<Angry2>()) machine.transition<Angry3>();
      else machine.transition<Angry1>();
      break;
    case Event::ButtonDoublePressed: machine.transition<Calm1>(); break;
    case Event::ButtonLongHeld: machine.transition<IdleA>(); break;
    case Event::Tick: break;
  }
}
inline void startHelmet() { machine.transition<IdleA>(); }
