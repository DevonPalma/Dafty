#pragma once
#include "LEDGeometry.h"

// Thomas Config

constexpr uint16_t LED_COUNT = 80;
constexpr uint8_t LED_PIN = 6;
constexpr uint8_t BUTTON_PIN = 2;
constexpr uint8_t LED_BRIGHTNESS = 100; // 0..255
constexpr uint8_t FPS = 30;             // frames per second

// Entire LED chain
using All = Strip<0, 80>;

// Full visors (4x10) - original mapping unchanged
using LeftVisor  = Matrix<0, 4, 10, MatrixOrder::Rows, true, false>;
using RightVisor = Matrix<40, 4, 10, MatrixOrder::Rows, true, true>;

// Upper sections (4x8)
using LeftUpper  = Matrix<0, 4, 8, MatrixOrder::Rows, true, false>;
using RightUpper = Matrix<48, 4, 8, MatrixOrder::Rows, true, true>;

// Lower exposed sections (4x2)
using LeftLower  = Matrix<32, 4, 2, MatrixOrder::Rows, true, false>;
using RightLower = Matrix<40, 4, 2, MatrixOrder::Rows, true, true>;

// Ears
using LeftEar  = Strip<0, 2>;
using RightEar = Strip<78, 2>;

// Combined sections
using BothVisors = Group<LeftVisor, RightVisor>;
using BothEars   = Group<LeftEar, RightEar>;
