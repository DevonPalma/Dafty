
#pragma once

#include "LEDShapes.h"
#include "FlashStorage.h"

// ==================================================
// HELMET SHAPES
//
// Shapes describe how LEDs are arranged.
// They do not create additional LED buffers.
//
// All definitions are stored in Flash.
// ==================================================

// Left 16-LED ring
const LEDShape LeftRingShape SD_FLASH =
  Ring(LeftRing, 0, true);

// Wokwi 48 x 8 matrix: ROW-major, progressive (not zigzag).
// If the real panel uses serpentine wiring, change false to true.
const LEDShape FaceMatrix SD_FLASH =
  Matrix(FacePanel, 48, 8, MatrixOrder::Rows, false);

// Right 16-LED ring
const LEDShape RightRingShape SD_FLASH =
  Ring(RightRing, 0, true);
