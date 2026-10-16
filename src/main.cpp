// APPLICATION ENTRY POINT
// This file connects the hardware, button events, and animation scheduler.
// For new patterns or playlists, edit Animations.h or AnimationPlaylist.h.
// For LED wiring and named regions, edit LEDSections.h.
// The loop stays nonblocking: call both update() methods every iteration.

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Debug.h"
#include "Button.h"
#include "LEDController.h"

LEDController leds;

#include "AnimationPlaylist.h"
#include "AnimationScheduler.h"

constexpr uint8_t BUTTON_PIN = 2;
Button button;
AnimationScheduler scheduler;
uint8_t selectedPlaylist = 0;

// Button controls: single click advances the scene, double click selects
// the next playlist, and a long press stops playback and clears the LEDs.
// These callbacks are registered in setup(); they run from button.update().
void nextScene() {
  if (scheduler.isRunning()) scheduler.next();
  else scheduler.play(selectedPlaylist);
}

void nextPlaylist() {
  selectedPlaylist = (selectedPlaylist + 1) % PLAYLIST_COUNT;
  scheduler.play(selectedPlaylist);
}

void stopShow() {
  scheduler.stop();
}

// Startup order matters: register physical strips before initializing them,
// then attach button handlers and start the first playlist.
void setup() {
  DEBUG_START();
  DEBUG_PRINTLN(F("===== Helmet NeoPixel Controller ====="));
  registerStrips();
  leds.begin();

  button.begin(BUTTON_PIN);
  button.onClick(nextScene);
  button.onDoubleClick(nextPlaylist);
  button.onLongPress(stopShow);

  scheduler.play(selectedPlaylist);
  DEBUG_PRINTLN(F("[SYSTEM] Ready"));
}

// Run both state machines frequently; do not add delay() here or button
// gestures and animation frames will become less responsive.
void loop() {
  button.update();
  scheduler.update();
}
