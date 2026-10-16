#pragma once
// Active-low button on pin 2. Non-blocking debounce and gesture recognition.
// Register callback functions in main.cpp. A single click fires after the
// double-click window; a long hold fires once while pressed.
#include <Arduino.h>

class Button
{
  uint8_t pin_ = 2;
  bool lastReading_ = HIGH;
  bool stable_ = HIGH;
  bool longFired_ = false;
  uint8_t clicks_ = 0;
  uint32_t changedAt_ = 0;
  uint32_t pressedAt_ = 0;
  uint32_t releasedAt_ = 0;
  void (*single_)() = nullptr;
  void (*double_)() = nullptr;
  void (*long_)() = nullptr;
  static const uint16_t DEBOUNCE_MS = 30;
  static const uint16_t DOUBLE_MS = 300;
  static const uint16_t LONG_MS = 700;

public:
  void begin(uint8_t pin = 2)
  {
    pin_ = pin;
    pinMode(pin_, INPUT_PULLUP);
    lastReading_ = stable_ = digitalRead(pin_);
    changedAt_ = millis();
  }
  void onClick(void (*fn)()) { single_ = fn; }
  void onDoubleClick(void (*fn)()) { double_ = fn; }
  void onLongPress(void (*fn)()) { long_ = fn; }

  void update()
  {
    const uint32_t now = millis();
    const bool reading = digitalRead(pin_);
    if (reading != lastReading_)
      changedAt_ = now;
    lastReading_ = reading;

    if (reading != stable_ && now - changedAt_ >= DEBOUNCE_MS)
    {
      stable_ = reading;
      if (stable_ == LOW)
      {
        pressedAt_ = now;
        longFired_ = false;
      }
      else if (!longFired_)
      {
        if (clicks_ < 2)
          ++clicks_;
        releasedAt_ = now;
      }
    }

    if (stable_ == LOW && !longFired_ && now - pressedAt_ >= LONG_MS)
    {
      longFired_ = true;
      clicks_ = 0;
      if (long_)
        long_();
    }

    if (stable_ == HIGH && clicks_ && now - releasedAt_ >= DOUBLE_MS)
    {
      const uint8_t count = clicks_;
      clicks_ = 0;
      if (count >= 2)
      {
        if (double_)
          double_();
      }
      else
      {
        if (single_)
          single_();
      }
    }
  }
};
