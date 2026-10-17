# Guy Manuel BLE-only controls update

See `helmets/guy_manuel/COMMANDS.md` for examples and command behavior.

Changes:
- Guy Manuel has no physical-button callbacks or button initialization; Thomas still does.
- `&State` selects a held state instantly, while `&Idle` resumes ambient cycling.
- Text and temporary expressions return to the most recently selected base state.
- Seven new expressions: Blink, Wink, Angry Eyes, Heartbeat, Glitch, Sleep and Loading (seven effects total).
- Unknown commands are rejected rather than printed.
- All special animations live inside Guy Manuel's `animations/SpecialAnimations.h`.

The original `guy_manuel.zip` was applied over the modular project baseline. For projects where Thomas has additional local edits, copy the Guy Manuel directory and merge only the small `src/main.cpp` / `platformio.ini` guard changes instead of replacing your whole workspace.

Validation: host C++11 syntax checks with Arduino-compatible stubs and parser assertions. Physical Nano 33 BLE and live Wokwi/PlatformIO compilation have not been performed.
