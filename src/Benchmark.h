#pragma once
#include <Arduino.h>

// =============================================================
// Super Dafty frame benchmark
// =============================================================
// Set to 0 to remove ALL timer reads, benchmark counters and prints
// from the animation scheduler at compile time.
#define BENCHMARK_ENABLED 0

// Report once per second. The Serial Monitor must be initialized at
// 115200 baud (DEBUG_START() in sketch.ino, or Serial.begin(115200)).
#define BENCHMARK_REPORT_MS 1000UL

#if BENCHMARK_ENABLED
class Benchmark {
  unsigned long windowStartMs = 0;
  uint16_t frames = 0;
  uint32_t renderSumUs = 0;
  uint32_t showSumUs = 0;
  uint32_t frameSumUs = 0;
  uint32_t slowestFrameUs = 0;

public:
  // Reset when changing playlists; samples never allocate heap memory.
  void reset() {
    windowStartMs = 0;
    frames = 0;
    renderSumUs = showSumUs = frameSumUs = slowestFrameUs = 0;
  }

  // Call AFTER leds.show(); serial printing is deliberately excluded
  // from measured rendering and transmission times.
  void record(uint32_t renderUs, uint32_t showUs, uint32_t frameUs) {
    const unsigned long nowMs = millis();
    if (windowStartMs == 0) windowStartMs = nowMs;

    ++frames;
    renderSumUs += renderUs;
    showSumUs += showUs;
    frameSumUs += frameUs;
    if (frameUs > slowestFrameUs) slowestFrameUs = frameUs;

    const unsigned long windowMs = nowMs - windowStartMs;
    if (windowMs < BENCHMARK_REPORT_MS) return;

    // Times in microseconds; FPS based on REAL elapsed wall time.
    // Multiply by 10 before dividing to retain one decimal digit.
    const uint32_t fps10 = (uint32_t)frames * 10000UL / windowMs;

    Serial.print(F("[BENCH] FPS "));
    Serial.print(fps10 / 10);
    Serial.print('.');
    Serial.print(fps10 % 10);
    Serial.print(F(" | Render "));
    Serial.print(renderSumUs / frames);
    Serial.print(F(" us | Show "));
    Serial.print(showSumUs / frames);
    Serial.print(F(" us | Work "));
    Serial.print(frameSumUs / frames);
    Serial.print(F(" us | Slowest "));
    Serial.print(slowestFrameUs);
    Serial.println(F(" us"));

    reset();
    // Start the next window from this print's end so serial output
    // overhead doesn't distort the *next* measured FPS window.
    windowStartMs = millis();
  }
};
#endif
