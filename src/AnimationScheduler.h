#pragma once
// NON-BLOCKING PLAYLIST ENGINE
// Scenes, playlists and effects remain in PROGMEM. Only playback state
// and, when enabled, lightweight benchmark counters live in SRAM.
// Each frame: read scene -> clear -> render effects -> show pixel buffer.
// Effects in a scene run simultaneously; later effects take precedence
// where they overlap.

#include "AnimationPlaylist.h"
#include "Debug.h"
#include "Benchmark.h"

class AnimationScheduler {
  uint8_t playlistIndex = 0;
  uint8_t sceneIndex = 0;
  bool running = false;
  unsigned long sceneStart = 0;
  unsigned long lastFrame = 0;
  static const unsigned long FRAME_MS = 33; // ~30 FPS target

#if BENCHMARK_ENABLED
  Benchmark benchmark;
#endif

  Playlist selected() const {
    return flashRead(&playlists[playlistIndex]);
  }

  Scene currentScene() const {
    Playlist p = selected();
    return flashRead(p.scenes + sceneIndex);
  }

  void advance(unsigned long at) {
    Playlist p = selected();
    if (++sceneIndex >= p.sceneCount) {
      if (p.repeat) sceneIndex = 0;
      else {
        running = false;
        leds.clear();
        leds.show();
        return;
      }
    }
    sceneStart = at;
  }

public:
  void play(uint8_t index) {
    if (index >= PLAYLIST_COUNT) return;
    playlistIndex = index;
    sceneIndex = 0;
    running = true;
    sceneStart = millis();
    lastFrame = sceneStart - FRAME_MS;
#if BENCHMARK_ENABLED
    benchmark.reset();
#endif
    DEBUG_PRINT(F("[PLAYLIST] Index: "));
    DEBUG_PRINTLN(index);
  }

  void next() {
    if (!running) return;
    advance(millis());
    lastFrame = millis() - FRAME_MS;
  }

  void stop() {
    running = false;
    leds.clear();
    leds.show();
  }

  bool isRunning() const { return running; }

  void update() {
    if (!running) return;
    const unsigned long now = millis();
    if ((unsigned long)(now - lastFrame) < FRAME_MS) return;
    lastFrame = now;

    Playlist p = selected();
    Scene scene = currentScene();
    for (uint8_t hops = 0; hops < p.sceneCount && running; ++hops) {
      if (scene.duration && (unsigned long)(now - sceneStart) < scene.duration) break;
      const unsigned long nextStart = scene.duration ? sceneStart + scene.duration : now;
      advance(nextStart);
      if (!running) return;
      scene = currentScene();
    }

    if (!scene.duration) return;
    unsigned long elapsed = now - sceneStart;
    if (elapsed >= scene.duration) {
      sceneStart = now;
      elapsed = 0;
    }

    // Timers surround ONLY pixel calculations and LED transmission.
    // No Serial.print() occurs between frameStartUs and frameEndUs.
#if BENCHMARK_ENABLED
    const uint32_t frameStartUs = micros();
#endif

    leds.clear();
    for (uint8_t i = 0; i < scene.effectCount; ++i) {
      FlashEffect effect = flashRead(scene.effects + i);
      if (!effect.duration) continue;
      const float progress = (float)(elapsed % effect.duration) / (float)effect.duration;
      renderFlashEffect(effect, progress);
    }

#if BENCHMARK_ENABLED
    const uint32_t showStartUs = micros();
#endif
    leds.show();
#if BENCHMARK_ENABLED
    const uint32_t frameEndUs = micros();
    const uint32_t renderUs = showStartUs - frameStartUs;
    const uint32_t showUs = frameEndUs - showStartUs;
    const uint32_t totalUs = frameEndUs - frameStartUs;
    benchmark.record(renderUs, showUs, totalUs);
#endif
  }
};
