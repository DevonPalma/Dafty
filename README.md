# Super Dafty: animation library edition

Arduino Uno R3, WS2812 chain on pin 6: left ring (16), 48x8 progressive row-major matrix (384), right ring (16). Button on pin 2 (INPUT_PULLUP). One Adafruit_NeoPixel framebuffer; no animation allocates an additional framebuffer or uses delay().

## Organization

- `src/LEDLayouts.h` compile-time Strip/Ring/Matrix/Group geometry.
- `src/animations/Common.h` tiny integer color, noise and timing helpers.
- `src/animations/Strip.h` Solid, Pulse, Fade, Wipe, Comet, Rainbow.
- `src/animations/Ring.h` Ring Chase, Dual Chase, Breathing, Sparkle.
- `src/animations/Matrix.h` Scanner, Equalizer, Rain, Glitch, Rainbow, Wave.
- `src/animations/Animations.h` includes the four headers above.
- `src/Animations.h` compatibility forwarding header; safe to remove if changing StateMachine include to `animations/Animations.h`.
- `src/StateMachine.h` compile-time state pairings, render functions, event handlers.
- `src/Button.h` debounced single/double/hold detection.
- `src/main.cpp` hardware initialization, event dispatch, and 30 FPS pacing.

## Usage in StateMachine.h

Keep using the existing functions:

```cpp
pulse<LeftRing>(Cyan, ctx.elapsedMs, 1700);
scanner<FaceMatrix>(Purple, ctx.elapsedMs, 2400, 2);
```

Or use any new animation directly in a state renderer:

```cpp
ringChase<LeftRing>(Cyan, ctx.elapsedMs, 1200, 4);
ringDualChase<RightRing>(Blue, Red, ctx.elapsedMs, 1800, 3);
ringSparkle<BothRings>(White, ctx.elapsedMs, 120, 4);
fade<FacePanel>(Blue, Cyan, ctx.elapsedMs, 1800);
comet<FacePanel>(Purple, ctx.elapsedMs, 2200, 6);
rainbow<FacePanel>(ctx.elapsedMs, 3000);
matrixEqualizer<FaceMatrix>(Cyan, Purple, ctx.elapsedMs, 4000, 16, 16);
matrixRain<FaceMatrix>(Green, ctx.elapsedMs, 130, 12, 3);
matrixGlitch<FaceMatrix>(Blue, White, ctx.elapsedMs, 90, 10);
matrixRainbow<FaceMatrix>(ctx.elapsedMs, 3500);
matrixWave<FaceMatrix>(Cyan, ctx.elapsedMs, 2400, 2);
```

All rendering functions write directly to the shared NeoPixel buffer. `main.cpp` clears it before calling the active state's renderer. Later calls overwrite pixels used by earlier calls. State rendering and transition behavior are deliberately unchanged in this version.

Matrix effects require a Matrix layout and are only evaluated on the matrix. Ring effects accept any 1D layout (including `BothRings`) and use the layout's logical order. Strip effects can also apply to Ring/Matrix/Group layouts in their logical 1D index order.

The equalizer is **synthetic**, not audio-reactive. Its bars interpolate between deterministic height targets and leave a small gap between bars. Adjust its bar count and beat count in your state calls.

## Change a layout

Edit only the aliases at the bottom of `src/LEDLayouts.h`, e.g. `FaceMatrix`'s `SERPENTINE` template parameter. Compile-time configuration results in specialized physical pixel access; no runtime mapping tables.

## Building / simulating

1. Open the project folder in VS Code with PlatformIO.
2. Build (`PlatformIO: Build`).
3. Start Wokwi with the included `wokwi.toml` and `diagram.json`.
4. Rebuild after modifying C++ source; Wokwi runs the resulting `.hex`.

No Serial benchmarking is enabled. Physical hardware needs an appropriately rated external 5 V LED power supply with common ground; do not run 416 LEDs from the Uno 5 V pin.
