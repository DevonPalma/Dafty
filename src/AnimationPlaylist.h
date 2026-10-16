#pragma once
// EDIT HERE: flash-resident effects, scenes, and playlists.
// The FX_* macros are documented in FlashAnimations.h. Targets are listed
// in the Target enum there. 1000ms = 1 second. Effects in a scene run
// together; scenes run one after another. Keep every array const SD_FLASH.


#include "FlashAnimations.h"

// ============================================================================
// SUPER DAFTY — LIGHT SHOWS
// ============================================================================
// EDIT THIS FILE to change what the helmet does.
//
// Hardware: LeftRing (16 LEDs) -> FacePanel (48 x 8) -> RightRing (16 LEDs).
//
// 1. EFFECTS:   Each FX_... line describes one animation.
// 2. SCENES:    Several effects play AT THE SAME TIME for a chosen duration.
// 3. PLAYLISTS: Scenes play IN ORDER. Double-click the button to switch shows.
//
// Durations are in milliseconds: 1000 = 1 second.
// Everything declared with SD_FLASH lives in program Flash, not permanent SRAM.
//
// Target::FacePanel    = all 384 matrix pixels in wiring order (1D effects)
// Target::FaceMatrix   = same pixels addressed as a 48 x 8 display (2D effects)
// Target::LeftRingShape / RightRingShape = circular ring animations
// Target::LeftRing / RightRing = linear effects on those 16 LEDs
//
// IMPORTANT: For a given scene, avoid overlapping targets unless intentional:
// later effects draw over earlier effects on the same LEDs.
// ============================================================================

#define COUNT_OF(array) (sizeof(array) / sizeof((array)[0]))

struct Scene {
  const FlashEffect* effects;
  uint8_t effectCount;
  unsigned long duration;
};

struct Playlist {
  const Scene* scenes;
  uint8_t sceneCount;
  bool repeat;
};

// ============================================================================
// PLAYLIST 1: SHOWCASE — bright, energetic helmet demonstration
// ============================================================================

// Opening: alternating ring beacons and a restrained face light.
const FlashEffect showBoot[] SD_FLASH = {
  FX_RINGCHASE(Target::LeftRingShape, Cyan,   1200),
  FX_PULSE(Target::FacePanel,       Blue,   2000),
  FX_RINGCHASE(Target::RightRingShape, Purple, 1200)
};

// Digital heartbeat: rapid color blocks while the two rings breathe.
const FlashEffect showHeartbeat[] SD_FLASH = {
  FX_PULSE(Target::LeftRing,        Purple, 1000),
  FX_NEONSTEPS(Target::FaceMatrix,    Cyan, Blue, 4000, 4, 6),
  FX_PULSE(Target::RightRing,       Cyan,   1000)
};

// Music-video-inspired: simulated visualizer, not real audio input.
const FlashEffect showEqualizer[] SD_FLASH = {
  FX_RINGCHASE(Target::LeftRingShape,  Cyan,   1600),
  FX_EQUALIZER(Target::FaceMatrix, Cyan, Purple, 4000, 12, 20),
  FX_RINGCHASE(Target::RightRingShape, Purple, 2000)
};

// Scanners sweep across all three pieces at independent rates.
const FlashEffect showScan[] SD_FLASH = {
  FX_SCANNER(Target::LeftRing,  Cyan,   2000, 3, 2),
  FX_SCANNER(Target::FaceMatrix, White,  4000, 3, 2),
  FX_SCANNER(Target::RightRing, Purple, 2000, 3, 2)
};

// Matrix-specific demo: a dot navigates the grid, rings orbit.
const FlashEffect showGeometry[] SD_FLASH = {
  FX_RINGCHASE(Target::LeftRingShape,  Blue,  1800),
  FX_MATRIXDOT(Target::FaceMatrix, White, 3600),
  FX_RINGCHASE(Target::RightRingShape, Cyan, 1800)
};

// Glitch finale: fast, deliberately chaotic flashes.
const FlashEffect showOverdrive[] SD_FLASH = {
  FX_PULSE(Target::LeftRing,    Red,  600),
  FX_GLITCH(Target::FaceMatrix,  Blue, White, 2400, 12, 16),
  FX_PULSE(Target::RightRing,   Red,  600)
};

// One cycle: ~30 seconds, then repeat.
const Scene showcaseScenes[] SD_FLASH = {
  {showBoot,      COUNT_OF(showBoot),       4000},
  {showBoot,      COUNT_OF(showBoot),       4000},
  {showEqualizer, COUNT_OF(showEqualizer),  8000},
  {showScan,      COUNT_OF(showScan),       4000},
  {showGeometry,  COUNT_OF(showGeometry),   3600},
  {showOverdrive, COUNT_OF(showOverdrive),  4800}
};

// ============================================================================
// PLAYLIST 2: AMBIENT — softer, slower effects for wearing the helmet
// ============================================================================

// Color breathing: calm ring pulses, gentle crossfade on the face.
const FlashEffect calmBreathe[] SD_FLASH = {
  FX_PULSE(Target::LeftRing,  Blue,   3000),
  FX_FADE(Target::FacePanel,   Blue, Purple, 6000),
  FX_PULSE(Target::RightRing, Purple, 3000)
};

// Slow rainbow face, slowly rotating rings.
const FlashEffect calmOrbit[] SD_FLASH = {
  FX_RINGCHASE(Target::LeftRingShape,  Cyan,   4000),
  FX_RAINBOW(Target::FaceMatrix, 8000, 0.5f, false),
  FX_RINGCHASE(Target::RightRingShape, Purple, 4000)
};

// Gentle wash of color across the helmet.
const FlashEffect calmWash[] SD_FLASH = {
  FX_FADE(Target::LeftRing,  Cyan, Blue,  6000),
  FX_FADE(Target::FacePanel,  Purple, Blue, 6000),
  FX_FADE(Target::RightRing, Blue, Cyan,  6000)
};

const Scene ambientScenes[] SD_FLASH = {
  {calmBreathe, COUNT_OF(calmBreathe),  6000},
  {calmOrbit,   COUNT_OF(calmOrbit),    8000},
  {calmWash,    COUNT_OF(calmWash),     6000}
};

// ============================================================================
// PLAYLIST 3: DIAGNOSTICS — check wiring and shape mapping
// ============================================================================

// Every region holds a unique color. Use this FIRST after rewiring.
const FlashEffect testSolid[] SD_FLASH = {
  FX_SOLID(Target::LeftRing,  Red,   5000),
  FX_SOLID(Target::FacePanel, Green, 5000),
  FX_SOLID(Target::RightRing, Blue,  5000)
};

// Checks the matrix X/Y mapping and ring directions.
const FlashEffect testShapes[] SD_FLASH = {
  FX_RINGCHASE(Target::LeftRingShape,  Cyan,  2000),
  FX_MATRIXDOT(Target::FaceMatrix, White, 4000),
  FX_RINGCHASE(Target::RightRingShape, Yellow, 2000)
};

// MATRIX CALIBRATION: four stages over 16 seconds.
// Expected on a correctly mapped, upright matrix:
//   0-4s  Red top-left, green top-right, blue bottom-left, white bottom-right
//   4-8s  Yellow row travels from top to bottom
//   8-12s Cyan column travels from left to right
//  12-16s Purple 8-LED physical wiring group travels through the matrix
// To isolate the matrix, rings are intentionally left off during this scene.
const FlashEffect testMatrix[] SD_FLASH = {
  FX_MATRIXTEST(Target::FaceMatrix, 16000)
};

const Scene diagnosticScenes[] SD_FLASH = {
  {testSolid,  COUNT_OF(testSolid),  5000},
  {testShapes, COUNT_OF(testShapes), 8000},
  {testMatrix, COUNT_OF(testMatrix), 16000}
};

// ============================================================================
// PLAYLIST REGISTRATION
// ============================================================================
// ORDER MATTERS: playlist 0 starts at power-on; double-click selects the next.
// true = loop when the last scene finishes; false = play once and stop.
// To add a new playlist, add its Scene array above, then register it below.

const Playlist playlists[] SD_FLASH = {
  {showcaseScenes,   COUNT_OF(showcaseScenes),   true},  // 0 — Showcase
  {ambientScenes,    COUNT_OF(ambientScenes),    true},  // 1 — Ambient
  {diagnosticScenes, COUNT_OF(diagnosticScenes), true}   // 2 — Diagnostics
};

constexpr uint8_t PLAYLIST_COUNT = COUNT_OF(playlists);
