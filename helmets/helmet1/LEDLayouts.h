#pragma once
// Helmet 1 only: pin assignments and geometry aliases.
#include "LEDGeometry.h"
constexpr uint16_t LED_COUNT = 416;
constexpr uint8_t LED_PIN = 6;
constexpr uint8_t BUTTON_PIN = 2;
using All = Strip<0, 416>;
using LeftRing = Ring<0, 16>;
using FacePanel = Strip<16, 384>;
using FaceMatrix = Matrix<16, 48, 8, MatrixOrder::Rows, false>; // progressive Wokwi
using RightRing = Ring<400, 16>;
using BothRings = Group<LeftRing, RightRing>;
