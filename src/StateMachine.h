#pragma once
// HELMET 1 SHOWCASE: 48x8 matrix between two 16-pixel rings.
// Every state is a compile-time pair of a renderer and its event handler.
// Button actions and all transition rules live in this header.
// No scene arrays, heap allocation, or extra LED framebuffers.
#include <Arduino.h>
// #include "LEDLayouts.h"
// #include "animations/Animations.h"

struct StateContext {
  uint32_t elapsedMs; // Resets whenever the state is entered.
  uint16_t progress;  // 0..1000 for timed states; 0 for held states.
};

enum class Event : uint8_t {
  Tick, ButtonPressed, ButtonDoublePressed, ButtonLongHeld
};

using StateRenderer = void (*)(const StateContext&);
using TransitionHandler = void (*)(Event);

template<StateRenderer RENDER, TransitionHandler HANDLE>
struct State {
  static void render(const StateContext& ctx) { RENDER(ctx); }
  static void handle(Event event) { HANDLE(event); }
};

class StateMachine {
  StateRenderer renderer_ = nullptr;
  TransitionHandler handler_ = nullptr;
  uint32_t enteredAt_ = 0;
  uint32_t durationMs_ = 0; // 0 = hold until a button event.

public:
  template<class NextState>
  void transition(uint32_t durationMs = 0) {
    renderer_ = &NextState::render;
    handler_ = &NextState::handle;
    enteredAt_ = millis();
    durationMs_ = durationMs;
  }

  template<class TState>
  bool is() const { return renderer_ == &TState::render; }

  bool timed() const { return durationMs_ != 0; }
  uint32_t elapsed() const { return millis() - enteredAt_; }
  bool finished() const { return timed() && elapsed() >= durationMs_; }

  void event(Event event) {
    if (handler_) handler_(event);
  }

  void render() const {
    if (!renderer_) return;
    const uint32_t e = elapsed();
    // All timed states here are < 4.3 seconds. 32-bit math is safe and
    // avoids expensive 64-bit division on the 8-bit Arduino Uno.
    const uint16_t p = durationMs_ == 0 ? 0 :
      (e >= durationMs_ ? 1000 : uint16_t(e * 1000UL / durationMs_));
    renderer_(StateContext{e, p});
  }
};

// This global is defined exactly once in main.cpp.
extern StateMachine machine;

// Keep all event hooks here. main.cpp only wires them to Button.h.
inline void buttonPressed()       { machine.event(Event::ButtonPressed); }
inline void buttonDoublePressed() { machine.event(Event::ButtonDoublePressed); }
inline void buttonLongHeld()      { machine.event(Event::ButtonLongHeld); }
