#pragma once
// Guy Manuel owns all BLE commands, state selection and transition policy.
// No physical button is required. All animation functions draw one frame.
#include <Arduino.h>
#include "LEDLayouts.h"
#include "Animations.h"
#include "StateMachine.h"
#ifdef DAFTY_ENABLE_BLE
#include "BluetoothText.h"
#include "commands/Commands.h"
#include "animations/TextAnimations.h"
#include "animations/SpecialAnimations.h"
#endif

inline void ambientDream(const StateContext &);
inline void ambientSpectrum(const StateContext &);
inline void ambientSweep(const StateContext &);
inline void partyPrism(const StateContext &);
inline void partyChaos(const StateContext &);
inline void alertSignal(const StateContext &);
inline void idleEvents(Event);
inline void otherEvents(Event);
#ifdef DAFTY_ENABLE_BLE
inline void textRender(const StateContext &);
inline void textEvents(Event);
inline void specialRender(const StateContext &);
inline void specialEvents(Event);
#endif
using IdleA = State<ambientDream, idleEvents>;
using IdleB = State<ambientSpectrum, idleEvents>;
using Sweep = State<ambientSweep, idleEvents>;
using Party = State<partyPrism, otherEvents>;
using PartyChaos = State<partyChaos, otherEvents>;
using Alert = State<alertSignal, otherEvents>;
#ifdef DAFTY_ENABLE_BLE
using Message = State<textRender, textEvents>;
using SpecialSequence = State<specialRender, specialEvents>;
#endif

inline void ambientDream(const StateContext &c)
{
  matrixWave<FaceMatrix>(Cyan, c.elapsedMs, 3600, 2);
  matrixRain<FaceMatrix>(Blue, c.elapsedMs, 210, 7, 2);
}
inline void ambientSpectrum(const StateContext &c)
{
  matrixEqualizer<FaceMatrix>(Cyan, Purple, c.elapsedMs, 4000, 16, 12);
}
inline void ambientSweep(const StateContext &c)
{
  matrixScanner<FaceMatrix>(White, c.elapsedMs, 2300, 3);
}
inline void partyPrism(const StateContext &c)
{
  matrixRainbow<FaceMatrix>(c.elapsedMs, 2100);
}
inline void partyChaos(const StateContext &c)
{
  matrixWave<FaceMatrix>(Cyan, c.elapsedMs, 1100, 3);
  matrixGlitch<FaceMatrix>(Purple, White, c.elapsedMs, 125, 11);
}

inline void alertSignal(const StateContext &c)
{
  // Repeat the entire sequence every 5 seconds.
  const uint32_t t = c.elapsedMs % 5000UL;

  // Flash red twice during the first 400 ms.
  if (t < 400)
  {
    if ((t / 100) % 2 == 0)
      solid<FacePanel>(Red);
  }
  else
  {
    // Scan continuously between flashes.
    matrixScanner<FaceMatrix>(Red, c.elapsedMs, 650, 1);
  }
}

// The only autonomous cycle is &Idle: Dream -> Spectrum -> Sweep.
inline void idleEvents(Event e)
{
  if (e != Event::Tick || !machine.finished())
    return;
  if (machine.is<IdleA>())
    machine.transition<IdleB>(4000);
  else if (machine.is<IdleB>())
    machine.transition<Sweep>(3800);
  else
    machine.transition<IdleA>(4000);
}
inline void otherEvents(Event) { /* Direct states hold until the next BLE command. */ }

#ifdef DAFTY_ENABLE_BLE
namespace guyControl
{
  // One saved base appearance, not a state stack. An interrupting command
  // replaces the previous temporary effect. Base mode survives messages.
  static guyCommands::Destination base = guyCommands::Destination::Idle;
  static guyCommands::Special activeSpecial = guyCommands::Special::None;

  inline void enterBase(guyCommands::Destination d)
  {
    using guyCommands::Destination;
    base = d;
    switch (d)
    {
    case Destination::Idle:
      machine.transition<IdleA>(4000);
      break;
    case Destination::Dream:
      machine.transition<IdleA>();
      break;
    case Destination::Spectrum:
      machine.transition<IdleB>();
      break;
    case Destination::Sweep:
      machine.transition<Sweep>();
      break;
    case Destination::Party:
      machine.transition<Party>();
      break;
    case Destination::Chaos:
      machine.transition<PartyChaos>();
      break;
    case Destination::Alert:
      machine.transition<Alert>();
      break;
    }
  }
  inline void resumeBase() { enterBase(base); }
}

inline void textRender(const StateContext &c)
{
  textAnimation<FaceMatrix>(c.elapsedMs);
}
inline void textEvents(Event e)
{
  if (e != Event::Tick)
    return;
  if (machine.elapsed() < textAnimationDuration<FaceMatrix>())
    return;
  if (guyCommands::current.hasNext)
    guyControl::enterBase(guyCommands::current.next);
  else
    guyControl::resumeBase();
}
inline void specialRender(const StateContext &c)
{
  using guyCommands::Special;
  switch (guyControl::activeSpecial)
  {
  case Special::WorkIt:
    workIt<FaceMatrix>(c.elapsedMs);
    break;
  case Special::HeartEyes:
    heartEyes<FaceMatrix>(c.elapsedMs);
    break;
  case Special::Blink:
    blinkEyes<FaceMatrix>(c.elapsedMs);
    break;
  case Special::Wink:
    winkEye<FaceMatrix>(c.elapsedMs);
    break;
  case Special::AngryEyes:
    angryEyes<FaceMatrix>(c.elapsedMs);
    break;
  case Special::Heartbeat:
    heartbeat<FaceMatrix>(c.elapsedMs);
    break;
  case Special::Glitch:
    glitchFace<FaceMatrix>(c.elapsedMs);
    break;
  case Special::Sleep:
    sleepingEyes<FaceMatrix>(c.elapsedMs);
    break;
  case Special::Loading:
    loadingFace<FaceMatrix>(c.elapsedMs);
    break;
  case Special::GreenHeart:
    greenHeart<FaceMatrix>(c.elapsedMs);
    break;
  case Special::None:
    break;
  }
}
inline void specialEvents(Event e)
{
  if (e == Event::Tick && machine.timed() && machine.finished())
    guyControl::resumeBase();
}

// Called by main.cpp when the shared BLE inbox has a new complete message.
// Unrecognized commands are ignored, leaving the current animation intact.
inline void helmetOnTextMessage()
{
  if (!guyCommands::parse(bluetoothText::message()))
    return;
  using guyCommands::Kind;
  switch (guyCommands::current.kind)
  {
  case Kind::DirectState:
    guyControl::enterBase(guyCommands::current.next);
    break;
  case Kind::Text:
    machine.transition<Message>();
    break;
  case Kind::SpecialEffect:
    guyControl::activeSpecial = guyCommands::current.special;
    machine.transition<SpecialSequence>(guyCommands::duration(guyControl::activeSpecial));
    break;
  case Kind::Invalid:
    break;
  }
}

inline void startHelmet()
{
#ifdef DAFTY_ENABLE_BLE
  guyControl::activeSpecial = guyCommands::Special::GreenHeart;
  machine.transition<SpecialSequence>();
#else
  machine.transition<IdleA>(4000);
#endif
}
#endif