#pragma once
#include "LEDGeometry.h"

// Guy Manuel: one 48x8 matrix, driven on pin 6.
constexpr uint16_t LED_COUNT = 384;
constexpr uint8_t LED_PIN = 6;
constexpr uint8_t BUTTON_PIN = 2;
constexpr uint8_t LED_BRIGHTNESS = 3; // 0..255
constexpr uint8_t FPS = 30;

using All = Strip<0, LED_COUNT>;
using FacePanel = All;
using FaceMatrix = Matrix<0, 48, 8, MatrixOrder::Columns, true, false>; // Column serpentine
