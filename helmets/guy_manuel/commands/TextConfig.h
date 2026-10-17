#pragma once
// Guy Manuel's currently selected text appearance. No drawing or BLE logic here.
#include "Color.h"
#include <stdint.h>
#include <string.h>

namespace guyText {
  enum class Draw : uint8_t { Scroll, Static, Blink };
  struct Config {
    char message[64];
    Color color;
    Draw draw;
  };
  // Only one message at a time. Commands update this before entering Message.
  static Config current = {{0}, Cyan, Draw::Scroll};

  inline void setMessage(const char* message) {
    if (!message) message = "";
    uint8_t i = 0;
    while (message[i] && i < sizeof(current.message) - 1) {
      current.message[i] = message[i]; ++i;
    }
    current.message[i] = '\0';
  }
}
