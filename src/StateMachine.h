#pragma once
#include <Arduino.h>
#include "Animations.h"

// The context passed to the active renderer.
struct StateContext {
    uint32_t elapsedMs;
    float progress;
};

// Every state is simply a rendering function.
using StateRenderer = void (*)(const StateContext&);

// Forward declarations allow states to transition to one another.
void idleA(const StateContext& ctx);
void idleB(const StateContext& ctx);
void party(const StateContext& ctx);

class StateMachine {
    StateRenderer active = nullptr;
    uint32_t enteredAt = 0;
    uint32_t durationMs = 0;

public:
    void transition(StateRenderer next, uint32_t duration = 0) {
        active = next;
        enteredAt = millis();
        durationMs = duration;
    }

    bool is(StateRenderer state) const {
        return active == state;
    }

    uint32_t elapsed() const {
        return millis() - enteredAt;
    }

    void render() const {
        if (!active) return;

        uint32_t elapsedMs = elapsed();

        float progress = 0.0f;
        if (durationMs) {
            progress = (float)elapsedMs / durationMs;
            if (progress > 1.0f) progress = 1.0f;
        }

        active({elapsedMs, progress});
    }
};

inline StateMachine machine;

// ------------------------------------------------------------
// STATE RENDERERS
// ------------------------------------------------------------

inline void idleA(const StateContext& ctx) {
    pulse<LeftRing>(Cyan, ctx.elapsedMs);
    scanner<FaceMatrix>(Purple, ctx.progress);
    pulse<RightRing>(Cyan, ctx.elapsedMs);
}

inline void idleB(const StateContext& ctx) {
    solid<LeftRing>(Blue);
    pulse<FaceMatrix>(Purple, ctx.elapsedMs);
    solid<RightRing>(Blue);
}

inline void party(const StateContext& ctx) {
    pulse<LeftRing>(Red, ctx.elapsedMs);
    scanner<FaceMatrix>(Cyan, ctx.progress);
    pulse<RightRing>(Red, ctx.elapsedMs);
}

// ------------------------------------------------------------
// TRANSITIONS
// ------------------------------------------------------------

// Automatic transitions, checked every loop.
inline void updateTransitions() {
    if (machine.is(idleA) && machine.elapsed() >= 4000) {
        machine.transition(idleB, 4000);
    }
    else if (machine.is(idleB) && machine.elapsed() >= 4000) {
        machine.transition(idleA, 4000);
    }
}

// Button callbacks.
inline void buttonPressed() {
    machine.transition(idleA);  // Hold indefinitely
}

inline void buttonDoublePressed() {
    machine.transition(party);  // Enter party mode
}

inline void buttonLongHeld() {
    machine.transition(idleB);  // Another state
}