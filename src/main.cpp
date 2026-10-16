#include <Arduino.h>
#include "LEDLayouts.h"
#include "AnimationScheduler.h"
#include "Button.h"
#include "Debug.h"

// The ONLY RGB framebuffer. Allocated internally by Adafruit_NeoPixel.
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
AnimationScheduler scheduler;
Button button;
uint8_t activePlaylist = 0;

void nextPlaylist()
{
  activePlaylist = (activePlaylist + 1) % PLAYLIST_COUNT;
  scheduler.play(activePlaylist);
}
void nextScene() { scheduler.next(); }
void togglePlayback()
{
  if (scheduler.isRunning())
    scheduler.stop();
  else
    scheduler.play(activePlaylist);
}

void setup()
{
#if DEBUG_ENABLED || BENCHMARK_ENABLED
  Serial.begin(115200);
#endif
  pixels.begin();
  pixels.setBrightness(100);
  clearPixels();
  showPixels();
  button.begin(2);
  button.onClick(nextScene);
  button.onDoubleClick(nextPlaylist);
  button.onLongPress(togglePlayback);
  scheduler.play(activePlaylist);
}
void loop()
{
  button.update();
  scheduler.update();
}
