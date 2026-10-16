# Super Dafty: two independent helmet state machines

Every helmet owns its complete `StateMachine.h`: the state runtime, renderers, button callbacks, and every transition rule. No state machine or state-mode implementation is shared under `src/`.

Shared `src/` contains only `main.cpp`, input/geometry/color utilities, and reusable animations. Each helmet has its own `LEDLayouts.h`, `StateMachine.h`, `diagram.json`, and `wokwi.toml`.

## Builds

- `pio run -e helmet1`
- `pio run -e helmet2`

Select the matching helmet's `wokwi.toml` in the Wokwi extension after building. Hardware upload: `pio run -e helmet1 -t upload` or `pio run -e helmet2 -t upload`.

## Where to edit

- `helmets/helmetN/LEDLayouts.h`: pins and LED shape definitions.
- `helmets/helmetN/StateMachine.h`: state implementation, visuals, animation timing, and button transitions.
- `src/animations/`: generic reusable animations.

The two state machines may evolve entirely differently without affecting each other. Shared `main.cpp` only expects `StateMachine machine`, `IdleA`, `Event::Tick`, and three `button...` callback functions.

Current Helmet 1 show: Dream/Spectrum/Sweep + Prism/Chaos/Alert. Helmet 2: Glow/Chase/Charging + Flow/Rush/Alert.
