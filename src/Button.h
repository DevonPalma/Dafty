
// BUTTON INPUT AND GESTURE DETECTION
// Uses INPUT_PULLUP: released = HIGH, pressed = LOW (button to ground).
// Register callbacks with onClick(), onDoubleClick(), and onLongPress(),
// then call update() frequently from loop(). No interrupts are needed.
// Gestures: a single click is reported after the double-click window;
// holding the button triggers a long press and cancels pending clicks.
// The debounce and gesture state machine below is intentionally unchanged.

#pragma once
#include <Arduino.h>
#include "Debug.h"

class Button
{
private:
  uint8_t pin;

  bool lastReading = HIGH;
  bool stableState = HIGH;
  bool longFired = false;

  uint8_t clicks = 0;

  unsigned long lastChange = 0;
  unsigned long pressStart = 0;
  unsigned long lastRelease = 0;

  void (*clickCallback)() = nullptr;
  void (*doubleCallback)() = nullptr;
  void (*longCallback)() = nullptr;

  // Timing thresholds (milliseconds): stable edge, double-click window,
  // and minimum hold duration. Adjust here only if input feel needs tuning.
  static const unsigned long DEBOUNCE_MS = 30;
  static const unsigned long DOUBLE_MS = 300;
  static const unsigned long LONG_MS = 700;

public:
  void begin(uint8_t buttonPin)
  {
    pin = buttonPin;
    pinMode(pin, INPUT_PULLUP);
    lastReading = stableState = digitalRead(pin);
  }

  void onClick(void (*callback)())
  {
    clickCallback = callback;
  }

  void onDoubleClick(void (*callback)())
  {
    doubleCallback = callback;
  }

  void onLongPress(void (*callback)())
  {
    longCallback = callback;
  }

  // 1. Debounce transitions using lastReading / stableState.
  // 2. Count completed short presses for single/double clicks.
  // 3. Fire a long press once while held, discarding pending clicks.
  // 4. Dispatch short-click gestures after DOUBLE_MS without new input.
  // Avoid delay() in the application loop so these checks run regularly.
  void update()
  {
    unsigned long now = millis();
    bool reading = digitalRead(pin);

    if (reading != lastReading)
      lastChange = now;

    lastReading = reading;

    if (now - lastChange >= DEBOUNCE_MS &&
        reading != stableState)
    {

      stableState = reading;

      if (stableState == LOW)
      {
        pressStart = now;
        longFired = false;
      }
      else if (!longFired)
      {
        clicks++;
        lastRelease = now;
      }
    }

    if (stableState == LOW &&
        !longFired &&
        now - pressStart >= LONG_MS)
    {

      longFired = true;
      clicks = 0;

      if (longCallback)
      {
        DEBUG_PRINTLN(F("[BUTTON] Long press"));
        longCallback();
      }
    }

    if (stableState == HIGH &&
        clicks > 0 &&
        now - lastRelease >= DOUBLE_MS)
    {

      uint8_t count = clicks;
      clicks = 0;

      if (count == 1 && clickCallback)
      {
        DEBUG_PRINTLN(F("[BUTTON] Single click"));
        clickCallback();
      }
      else if (count >= 2 && doubleCallback)
      {
        DEBUG_PRINTLN(F("[BUTTON] Double click"));
        doubleCallback();
      }
    }
  }
};