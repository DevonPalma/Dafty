#pragma once
// Guy Manuel's BLE command language. No states, NeoPixels or BLE transport here.
#include "TextConfig.h"
#include <stdint.h>
#include <string.h>

namespace guyCommands {
  enum class Destination : uint8_t { Idle, Dream, Spectrum, Sweep, Party, Chaos, Alert };
  enum class Special : uint8_t {
    None, WorkIt, HeartEyes, Blink, Wink, AngryEyes, Heartbeat, Glitch, Sleep, Loading, GreenHeart
  };
  enum class Kind : uint8_t { Invalid, Text, DirectState, SpecialEffect };
  enum class Error : uint8_t { None, UnknownCommand, MissingText };
  struct Action {
    Kind kind;
    Destination next;
    bool hasNext;
    Special special;
    Error error;
  };
  static Action current = {Kind::Invalid, Destination::Idle, false, Special::None, Error::None};

  inline bool equals(const char* a, const char* b, size_t n) {
    if (strlen(b) != n) return false;
    for (size_t i = 0; i < n; ++i) {
      char x=a[i], y=b[i];
      if (x >= 'A' && x <= 'Z') x += ('a'-'A');
      if (y >= 'A' && y <= 'Z') y += ('a'-'A');
      if (x != y) return false;
    }
    return true;
  }

  inline bool colorName(const char* s, size_t n, Color& c) {
    if (equals(s,"red",n)) c=Red;
    else if (equals(s,"green",n)) c=Green;
    else if (equals(s,"blue",n)) c=Blue;
    else if (equals(s,"cyan",n)) c=Cyan;
    else if (equals(s,"purple",n)) c=Purple;
    else if (equals(s,"yellow",n)) c=Yellow;
    else if (equals(s,"white",n)) c=White;
    else if (equals(s,"black",n)) c=Black;
    else if (equals(s,"orange",n)) c=RGB(255,90,0);
    else if (equals(s,"pink",n)) c=RGB(255,20,110);
    else return false;
    return true;
  }
  inline bool destinationName(const char* s, size_t n, Destination& d) {
    if (equals(s,"idle",n) || equals(s,"ambient",n)) d=Destination::Idle;
    else if (equals(s,"dream",n)) d=Destination::Dream;
    else if (equals(s,"spectrum",n)) d=Destination::Spectrum;
    else if (equals(s,"sweep",n)) d=Destination::Sweep;
    else if (equals(s,"party",n)) d=Destination::Party;
    else if (equals(s,"chaos",n)) d=Destination::Chaos;
    else if (equals(s,"alert",n)) d=Destination::Alert;
    else return false;
    return true;
  }
  inline bool drawName(const char* s, size_t n, guyText::Draw& draw) {
    if (equals(s,"scroll",n)) draw=guyText::Draw::Scroll;
    else if (equals(s,"static",n) || equals(s,"hold",n)) draw=guyText::Draw::Static;
    else if (equals(s,"blink",n)) draw=guyText::Draw::Blink;
    else return false;
    return true;
  }
  inline bool specialName(const char* s, size_t n, Special& e) {
    if (equals(s,"work_it",n)) e=Special::WorkIt;
    else if (equals(s,"heart_eyes",n) || equals(s,"hearts",n)) e=Special::HeartEyes;
    else if (equals(s,"blink",n)) e=Special::Blink;
    else if (equals(s,"wink",n)) e=Special::Wink;
    else if (equals(s,"angry_eyes",n)) e=Special::AngryEyes;
    else if (equals(s,"heartbeat",n)) e=Special::Heartbeat;
    else if (equals(s,"glitch",n)) e=Special::Glitch;
    else if (equals(s,"sleep",n)) e=Special::Sleep;
    else if (equals(s,"loading",n)) e=Special::Loading;
    else if (equals(s,"green_heart",n) || equals(s,"greenheart",n)) e=Special::GreenHeart;
    else return false;
    return true;
  }
  inline bool persistent(Special e) {
    return e==Special::Sleep || e==Special::AngryEyes || e==Special::Loading || e==Special::GreenHeart;
  }
  inline uint32_t duration(Special e) {
    switch (e) {
      case Special::WorkIt: return 7200;
      case Special::HeartEyes: return 6000;
      case Special::Blink: return 1200;
      case Special::Wink: return 1600;
      case Special::Heartbeat: return 4000;
      case Special::Glitch: return 1800;
      default: return 0; // Persistent expressions.
    }
  }

  // Parse into temporary values, committing only after validation. Unknown
  // commands never display as text or unexpectedly interrupt the show.
  inline bool parse(const char* input) {
    Action a={Kind::Invalid,Destination::Idle,false,Special::None,Error::UnknownCommand};
    if (!input) { current=a; return false; }
    const char* p=input;
    while (*p==' ') ++p;
    if (!*p) { current=a; return false; }
    if (*p=='^') {
      ++p;
      const char* end=p;
      while (*end && *end!=' ') ++end;
      while (*end==' ') ++end;
      if (*end || !specialName(p, strlen(p), a.special)) { current=a; return false; }
      a.kind=Kind::SpecialEffect; a.error=Error::None;
      current=a; return true;
    }
    Color color=Cyan;
    guyText::Draw draw=guyText::Draw::Scroll;
    bool anyPrefix=false;
    while (*p=='$' || *p=='&' || *p=='*') {
      const char prefix=*p++;
      const char* token=p;
      while (*p && *p!=' ') ++p;
      const size_t n=size_t(p-token);
      bool ok=false;
      if (n) {
        if (prefix=='$') ok=colorName(token,n,color);
        if (prefix=='&') { ok=destinationName(token,n,a.next); if(ok) a.hasNext=true; }
        if (prefix=='*') ok=drawName(token,n,draw);
      }
      if (!ok) { current=a; return false; }
      anyPrefix=true;
      while (*p==' ') ++p;
    }
    // A command-looking token after the prefix is an error, not literal text.
    if (*p=='$' || *p=='&' || *p=='*' || *p=='^') { current=a; return false; }
    if (!*p) {
      if (a.hasNext && anyPrefix) {
        a.kind=Kind::DirectState; a.error=Error::None;
        current=a; return true;
      }
      a.error=Error::MissingText; current=a; return false;
    }
    guyText::current.color=color;
    guyText::current.draw=draw;
    guyText::setMessage(p);
    a.kind=Kind::Text; a.error=Error::None;
    current=a;
    return true;
  }
}
