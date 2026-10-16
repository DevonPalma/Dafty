# Super Dafty: Flash-backed NeoPixel helmet

**Target:** Arduino Uno R3, 416 LEDs, pin 6, button pin 2.

**Physical data chain:** left 16-pixel ring → 48×8 matrix (384 pixels) → right 16-pixel ring.

## Why this version exists

The previous version constructed every Effect in SRAM, even when its scene wasn't playing. On an Uno with 2,048 bytes of SRAM, the 416-pixel NeoPixel RGB buffer alone needs **1,248 bytes**. This version stores **all effect descriptors, scene tables, playlist tables, section spans, and shape definitions in program Flash** (AVR `PROGMEM`).

Only the current scene/effect is read into small **temporary stack objects** when a frame is rendered. Scheduler state, button state, NeoPixel objects, the changing LED buffer, and stack still require SRAM. No extra matrix framebuffer is allocated. **Do not assume the project fits until Wokwi or a real AVR compile and runtime memory test confirms it.**

## How to change the show

Edit **`AnimationPlaylist.h`**. A group of effects runs at the same time:

```cpp
const FlashEffect example[] SD_FLASH = {
  FX_SOLID(Target::LeftRing, Red, 2000),
  FX_MATRIXDOT(Target::FaceMatrix, Cyan, 3000),
  FX_RINGCHASE(Target::RightRingShape, Purple, 1500)
};
```

A scene chooses effects and its total duration:

```cpp
const Scene exampleScenes[] SD_FLASH = {
  {example, COUNT_OF(example), 8000}
};
```

Add a playlist entry:

```cpp
const Playlist playlists[] SD_FLASH = {
  {exampleScenes, COUNT_OF(exampleScenes), true}
};
```

**Use `const ... SD_FLASH`**, not plain mutable arrays. All macro parameters must be compile-time constants. The old `.on(...).speed(...)` builders are no longer used in playlists.

## Available targets

- `Target::All` — all 416 pixels
- `Target::LeftRing`, `Target::RightRing`, `Target::BothRings` — raw linear ring indices
- `Target::FacePanel` — matrix LEDs as a **linear** section
- `Target::LeftRingShape`, `Target::RightRingShape` — circular shape mapping
- `Target::FaceMatrix` — **48×8 column-major zigzag** shape mapping

The actual matrix wiring may require adjusting `ShapeDefinitions.h` flags if 2D patterns are scrambled.

## Available effect macros

| Macro | Arguments after target | What it does |
|---|---|---|
| `FX_SOLID` | color, milliseconds | Solid color |
| `FX_FADE` | from, to, ms | Crossfade between colors |
| `FX_PULSE` | color, ms | Brightness rise and fall |
| `FX_WIPE` | color, ms | Fill a section along its linear order |
| `FX_RAINBOW` | ms, rotations, reverse | Moving rainbow; reverse is `true`/`false` |
| `FX_COMET` | color1, color2, ms, speed1, speed2, tail | Opposing comets; speeds are laps/cycle |
| `FX_EQUALIZER` | lowColor, highColor, ms, bars, beats | Pseudo-music equalizer bars |
| `FX_SCANNER` | color, ms, trail, sweeps | Back-and-forth scanner |
| `FX_GLITCH` | baseColor, flashColor, ms, blocks, changes | Flickering blocks |
| `FX_NEONSTEPS` | color1, color2, ms, width, cycles | Scrolling neon blocks |
| `FX_MATRIXDOT` | color, ms | Dot moving through a **matrix shape** |
| `FX_RINGCHASE` | color, ms | Rotating dot on a **ring shape** |

Examples:

```cpp
FX_RAINBOW(Target::FacePanel, 5000, 2.0f, false),
FX_COMET(Target::LeftRing, Blue, Purple, 4000, 1.0f, 0.5f, 6),
FX_GLITCH(Target::FacePanel, Blue, White, 2000, 14, 32),
FX_EQUALIZER(Target::FacePanel, Cyan, Purple, 4000, 12, 10)
```

Durations are milliseconds (`1000 = 1 second`). Use colors `Red`, `Blue`, `Green`, `Purple`, `Yellow`, `Cyan`, `White`, `Black`, or a constant `Color{r,g,b}`. For `FX_MATRIXDOT` and `FX_RINGCHASE`, use the corresponding shape target, not the linear section.

## File guide

- `AnimationPlaylist.h`: **EDIT HERE** for effects, scenes, playlists.
- `LEDSections.h`: physical strip lengths and logical LED ranges.
- `ShapeDefinitions.h`: layout of rings and matrix.
- `FlashAnimations.h`: Flash effect descriptors and `FX_...` macros.
- `FlashStorage.h`: portable `PROGMEM` access.
- `AnimationScheduler.h`: scheduler reads Flash definitions at playback.
- `Animations.h`: renderer implementations. The legacy builders remain in this file for compatibility, but **are not used** to create playlist arrays.
- `LEDController.h`, `LEDShapes.h`, `AnimationTypes.h`: internal implementation.
- `sketch.ino`: startup, button bindings, main loop.

## Button controls

- Single click: next scene (or restart if stopped)
- Double click: next playlist
- Long press: stop and clear the LEDs

## Memory and wiring cautions

- **Flash is not writable SRAM.** AVR code must use `memcpy_P`/`pgm_read_*` to access `PROGMEM` objects. This version uses `flashRead()` and Flash span mapping.
- **NeoPixel still allocates 1,248 bytes of SRAM** for 416 RGB pixels. You'll need additional free SRAM for stack and interrupt operation.
- The included example diagram powers pixels from the Uno 5V rail for simulation. **Do not power 416 WS2812 LEDs at full white from an Uno 5V pin on real hardware.** Use a properly sized external 5V supply and common ground.
- This refactor was compiled with a desktop C++11 Arduino stub. It has **not** yet been compiled with AVR GCC or verified in Wokwi/hardware. If the Uno remains unstable, inspect the actual binary's SRAM use and free-stack margin.
- BLE is not included. A standard Uno R3 has no built-in BLE.

## Extending animations

To add a new effect, define a new `EffectKind`, Flash macro, and case in `renderFlashEffect()` to unpack the Flash parameters into the existing renderer's settings. The `FlashParams` struct holds up to two colors, two floats, two bytes and a boolean **per effect descriptor in Flash**. This is a simple interim schema for existing effects, not unlimited arbitrary custom configuration. Changing these fields affects Flash size, not permanent SRAM per definition.

## Maintaining the code

- **AnimationPlaylist.h:** Edit Flash-backed effects, scene lengths, and playlist order.
- **LEDSections.h:** Physical strip pins and logical LED spans.
- **ShapeDefinitions.h:** Matrix/ring orientation and layout. Wokwi is currently row-major progressive.
- **Animations.h:** Renderer algorithms only. The legacy runtime builder API has been removed.
- **FlashAnimations.h:** FX_* macros, effect kind dispatch and target resolution. Add a dispatch case when introducing a new renderer.
- **AnimationTypes.h:** Internal stack-only effect adapter. Never put Effect arrays in playlists.
- **LEDController.h / LEDShapes.h:** Physical mapping and geometry helpers.
- **AnimationScheduler.h:** Frame timing; keep delay() out of effects and loop().

### Known limitations

The existing Equalizer, Scanner, Glitch, NeonSteps and Rainbow algorithms
operate on **linear LED order**, even when that order happens to be a 48x8
matrix. Their horizontal-band appearance is expected. `MatrixDot` and
`MatrixTest` use actual X/Y coordinates. Proper 2D effect variants would be
new features, not a consequence of the cleanup.

**Do not connect 416 real NeoPixels directly to an Uno 5V pin.** Use a suitably
sized 5V power supply and a shared ground. Wokwi's power wiring is only
a simulation.

### Debugging order

1. Use the Diagnostics playlist solid-color scene to confirm ring/matrix ranges.
2. Use `FX_MATRIXTEST(Target::FaceMatrix, 16000)` to verify four corners and moving rows/columns.
3. Verify `ShapeDefinitions.h` matches the Wokwi layout selector (progressive vs serpentine).
4. Only then try the full Showcase playlist. If the simulation goes dark after adding effects, check free SRAM on the Uno.

### Compilation confidence

The cleaned version passed a C++11 compilation and basic setup/loop smoke test
using local Arduino/NeoPixel-compatible stubs. This verifies C++ structure, not
AVR PROGMEM placement, pin timing, or electrical behavior. Run it in Wokwi
and on the target board before treating the refactor as hardware-tested.
