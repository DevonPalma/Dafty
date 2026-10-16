#pragma once
#include <Arduino.h>
struct Color { uint8_t r, g, b; };
constexpr Color Red={255,0,0}, Green={0,255,0}, Blue={0,0,255};
constexpr Color Purple={160,0,255}, Yellow={255,200,0}, Cyan={0,255,255};
constexpr Color White={255,255,255}, Black={0,0,0};
inline Color RGB(uint8_t r,uint8_t g,uint8_t b) { return {r,g,b}; }
