#pragma once
// Super Dafty: compile-time state bindings and event-driven transitions.
// A state pairs two ordinary functions: one renders LEDs, the other handles events.
// All state-specific transitions and button callback functions live here.
#include <Arduino.h>
#include "animations/Animations.h"

struct StateContext
{
  uint32_t elapsedMs; // Milliseconds since entering this state.
  uint16_t progress;  // 0..1000 for timed states; 0 if indefinite.
};

enum class Event : uint8_t
{
  Tick, // Called every main-loop iteration (not just every frame).
  ButtonPressed,
  ButtonDoublePressed,
  ButtonLongHeld
};

using StateRenderer = void (*)(const StateContext &);
using TransitionHandler = void (*)(Event);

// Bind render + event behavior at compile time. This creates no state object.
template <StateRenderer RENDER, TransitionHandler HANDLE>
struct State
{
  static void render(const StateContext &ctx) { RENDER(ctx); }
  static void handle(Event event) { HANDLE(event); }
};

class StateMachine
{
  StateRenderer currentRender_ = nullptr;
  TransitionHandler currentHandler_ = nullptr;
  uint32_t enteredAt_ = 0;
  uint32_t durationMs_ = 0; // Zero means no automatic timeout.

public:
  template <class TState>
  void transition(uint32_t durationMs = 0)
  {
    // Both addresses are selected at compile time, no runtime state switch.
    currentRender_ = &TState::render;
    currentHandler_ = &TState::handle;
    enteredAt_ = millis();
    durationMs_ = durationMs;
  }

  template <class TState>
  bool is() const { return currentRender_ == &TState::render; }

  uint32_t elapsed() const { return millis() - enteredAt_; }
  bool timed() const { return durationMs_ != 0; }
  bool finished() const { return timed() && elapsed() >= durationMs_; }

  void event(Event e)
  {
    if (currentHandler_)
      currentHandler_(e);
  }

  void render() const
  {
    if (!currentRender_)
      return;
    const uint32_t e = elapsed();
    uint16_t progress = 0;
    if (durationMs_)
    {
      // 64-bit intermediate keeps progress correct for long durations.
      progress = e >= durationMs_ ? 1000
                                  : uint16_t((uint64_t(e) * 1000u) / durationMs_);
    }
    currentRender_(StateContext{e, progress});
  }
};

// Defined exactly once in main.cpp. Works in C++11 on the Arduino Uno.
extern StateMachine machine;

// Forward declarations are required because states can refer to one another.
inline void idleA(const StateContext &);
inline void idleB(const StateContext &);
inline void party(const StateContext &);
inline void idleATransitions(Event);
inline void idleBTransitions(Event);
inline void partyTransitions(Event);

// Named state TYPES: no enum registry, tables, or per-state SRAM allocation.
using IdleA = State<idleA, idleATransitions>;
using IdleB = State<idleB, idleBTransitions>;
using Party = State<party, partyTransitions>;

// ------------------------------------------------------------
// RENDERERS: all animations in a state draw into the shared LED buffer.
// main.cpp clears once before calling machine.render().
// ------------------------------------------------------------
inline void idleA(const StateContext &ctx)
{
  pulse<LeftRing>(Cyan, ctx.elapsedMs, 1700);
  scanner<FaceMatrix>(Purple, ctx.elapsedMs, 2400, 2);
  pulse<RightRing>(Cyan, ctx.elapsedMs, 1700);
}

inline void idleB(const StateContext &ctx)
{
  solid<LeftRing>(Blue);
  pulse<FaceMatrix>(Purple, ctx.elapsedMs, 1800);
  solid<RightRing>(Blue);
}

inline void party(const StateContext &ctx)
{
  pulse<LeftRing>(Red, ctx.elapsedMs, 650);
  scanner<FaceMatrix>(Cyan, ctx.elapsedMs, 800, 3);
  pulse<RightRing>(Red, ctx.elapsedMs, 650);
}

// ------------------------------------------------------------
// TRANSITIONS: each state's outgoing edges live alongside its renderer.
// A timed state advances on Tick only after its duration ends.
// Indefinite states never finish, so single-click can pin A or B.
// ------------------------------------------------------------
inline void idleATransitions(Event e)
{
  switch (e)
  {
  case Event::Tick:
    if (machine.finished())
      machine.transition<IdleB>(4000);
    break;
  case Event::ButtonPressed:
    machine.transition<IdleA>(); // Pin Idle A indefinitely.
    break;
  case Event::ButtonDoublePressed:
    machine.transition<Party>();
    break;
  case Event::ButtonLongHeld:
    machine.transition<IdleA>(4000); // Restart automatic loop.
    break;
  }
}

inline void idleBTransitions(Event e)
{
  switch (e)
  {
  case Event::Tick:
    if (machine.finished())
      machine.transition<IdleA>(4000);
    break;
  case Event::ButtonPressed:
    machine.transition<IdleB>(); // Pin Idle B indefinitely.
    break;
  case Event::ButtonDoublePressed:
    machine.transition<Party>();
    break;
  case Event::ButtonLongHeld:
    machine.transition<IdleA>(4000);
    break;
  }
}

inline void partyTransitions(Event e)
{
  switch (e)
  {
  case Event::ButtonPressed:
    machine.transition<IdleA>(); // Leave party; pin Idle A.
    break;
  case Event::ButtonLongHeld:
    machine.transition<IdleA>(4000); // Resume A <-> B loop.
    break;
  case Event::ButtonDoublePressed:
  case Event::Tick:
    break; // Party holds indefinitely by default.
  }
}

// Button.h callbacks accept void(*)(), so translate them here.
inline void buttonPressed() { machine.event(Event::ButtonPressed); }
inline void buttonDoublePressed() { machine.event(Event::ButtonDoublePressed); }
inline void buttonLongHeld() { machine.event(Event::ButtonLongHeld); }
