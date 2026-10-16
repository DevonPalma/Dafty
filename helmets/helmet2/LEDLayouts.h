#pragma once
// Helmet 2 only: pin assignments and geometry aliases.
#include "LEDGeometry.h"
constexpr uint16_t LED_COUNT = 80;
constexpr uint8_t LED_PIN = 6;
constexpr uint8_t BUTTON_PIN = 2;
using All        = Strip<0, 80>;
using LeftVisor  = Strip<0, 20>;
using RightVisor = Strip<20, 20>;
using LeftEar    = Strip<40, 10>;
using RightEar   = Strip<50, 10>;
using Rear       = Strip<60, 20>;
using BothVisors = Group<LeftVisor, RightVisor>;
using BothEars   = Group<LeftEar, RightEar>;
using BothSides  = Group<BothVisors, BothEars>;
