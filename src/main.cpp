#include <Arduino.h>
#include "LEDLayouts.h"
#include "Button.h"
#include "StateMachine.h"

// Single shared NeoPixel buffer and one state machine.
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
StateMachine machine;
Button button;

static const uint16_t FRAME_MS = 33; // ~30 FPS.

void setup()
{
  pixels.begin();
  pixels.setBrightness(100);
  clearPixels();
  showPixels();

  button.begin(BUTTON_PIN);
  button.onClick(buttonPressed);
  button.onDoubleClick(buttonDoublePressed);
  button.onLongPress(buttonLongHeld);

  machine.transition<IdleA>(4000);
}

void loop()
{
  // Handle input and time-based transitions as often as possible.
  button.update();
  machine.event(Event::Tick);

  // Only LED rendering is frame-limited.
  static uint32_t lastFrame = millis();
  const uint32_t now = millis();
  if (now - lastFrame < FRAME_MS)
    return;
  lastFrame = now;

  clearPixels();
  machine.render();
  showPixels();
}
