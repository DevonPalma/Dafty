#pragma once
// Guy Manuel's command language. No BLE, LEDs, or state-machine dependencies.
// $Color &NextState *DrawAnimation Message  OR  ^SpecialCommand
#include "TextConfig.h"
#include <stdint.h>
#include <string.h>

namespace guyCommands {
  enum class Destination : uint8_t { Dream, Spectrum, Sweep, Party, Chaos, Alert };
  using Draw = guyText::Draw;
  enum class Special : uint8_t { None, WorkIt, HeartEyes };

  struct Action {
    Destination next;
    Special special;
  };
  // State routing only; visible text/color/drawing live in guyText::current.
  static Action current = {Destination::Dream, Special::None};

  inline bool equalIgnoreCase(const char* a, const char* b, uint8_t n) {
    if (strlen(b) != n) return false;
    for (uint8_t i = 0; i < n; ++i) {
      char x = a[i], y = b[i];
      if (x >= 'A' && x <= 'Z') x += 'a' - 'A';
      if (y >= 'A' && y <= 'Z') y += 'a' - 'A';
      if (x != y) return false;
    }
    return true;
  }

  inline bool colorName(const char* s, uint8_t n, Color& result) {
    struct Entry { const char* name; Color color; };
    const Entry colors[] = {
      {"red", Red}, {"green", Green}, {"blue", Blue},
      {"cyan", Cyan}, {"purple", Purple}, {"yellow", Yellow},
      {"white", White}, {"black", Black},
      {"orange", {255, 90, 0}}, {"pink", {255, 20, 110}}
    };
    for (const Entry& e : colors) {
      if (equalIgnoreCase(s, e.name, n)) { result = e.color; return true; }
    }
    return false;
  }

  inline bool destinationName(const char* s, uint8_t n, Destination& result) {
    const char* const names[] = {"dream", "spectrum", "sweep", "party", "chaos", "alert"};
    for (uint8_t i = 0; i < 6; ++i) {
      if (equalIgnoreCase(s, names[i], n)) {
        result = static_cast<Destination>(i); return true;
      }
    }
    if (equalIgnoreCase(s, "idle", n) || equalIgnoreCase(s, "ambient", n)) {
      result = Destination::Dream; return true;
    }
    return false;
  }

  inline bool drawName(const char* s, uint8_t n, Draw& result) {
    if (equalIgnoreCase(s, "scroll", n)) { result = Draw::Scroll; return true; }
    if (equalIgnoreCase(s, "static", n) || equalIgnoreCase(s, "hold", n)) { result = Draw::Static; return true; }
    if (equalIgnoreCase(s, "blink", n)) { result = Draw::Blink; return true; }
    return false;
  }

  inline void parse(const char* input) {
    Action action = {Destination::Dream, Special::None};
    guyText::current.color = Cyan;
    guyText::current.draw = Draw::Scroll;
    guyText::setMessage("");
    if (!input) { current = action; return; }

    const char* p = input;
    while (*p == ' ') ++p;
    if (*p == '^') {
      const char* name = p + 1;
      const size_t size = strlen(name);
      if (size <= 63 && equalIgnoreCase(name, "work_it", uint8_t(size)))
        action.special = Special::WorkIt;
      else if (size <= 63 && (equalIgnoreCase(name, "heart_eyes", uint8_t(size)) ||
                              equalIgnoreCase(name, "hearts", uint8_t(size))))
        action.special = Special::HeartEyes;
      if (action.special != Special::None) { current = action; return; }
      // Unknown specials are printed as ordinary text.
    } else {
      // Parse any number of known leading prefixes, in any order.
      while (*p == '$' || *p == '&' || *p == '*') {
        const char prefix = *p;
        const char* token = p + 1;
        const char* end = token;
        while (*end && *end != ' ') ++end;
        const size_t span = size_t(end - token);
        if (!span || span > 63) break;
        const uint8_t n = uint8_t(span);
        bool ok = false;
        if (prefix == '$') ok = colorName(token, n, guyText::current.color);
        if (prefix == '&') ok = destinationName(token, n, action.next);
        if (prefix == '*') ok = drawName(token, n, guyText::current.draw);
        if (!ok) break;
        p = end;
        while (*p == ' ') ++p;
      }
    }
    guyText::setMessage(p);
    current = action;
  }
}
