# Helmet Setup

Each helmet lives in `helmets/<name>/` and has its own `LEDLayouts.h`, `States.h`, `diagram.json`, and `wokwi.toml`. The animation engine in `src/` is shared.

## 1. Build a helmet

Open the project in VS Code with PlatformIO installed. From the project root:

```bash
pio run -e helmet1       # or: helmet2
```

To add a helmet, copy an existing `helmets/<name>/` folder, rename it, and add a matching `[env:<name>]` entry to `platformio.ini` using that helmet's include path and build selection.

## 2. Flash to Arduino

Connect the correct Arduino Uno over USB, then run:

```bash
pio run -e helmet1 -t upload    # or: helmet2
```

If upload detection fails, find the Arduino's COM port in Device Manager and set `upload_port` in its PlatformIO environment.

**Hardware:** Power the LED strips from an appropriately rated external 5 V supply, with a shared ground between the supply and Arduino. Don't power a large LED chain through USB.

## 3. Simulate in Wokwi

1. Build the helmet using `pio run -e helmet1` (or `helmet2`).
2. In VS Code, run **Wokwi: Select Config File** and choose `helmets/<name>/wokwi.toml`.
3. Run **Wokwi: Start Simulator**. It uses that helmet's `diagram.json` and compiled firmware.

Rebuild after changing code. Ensure `wokwi.toml` points to `.pio/build/<name>/firmware.hex` and `.elf` (paths relative to that TOML file).

## 4. Configure

| File | Change here |
|---|---|
| `helmets/<name>/LEDLayouts.h` | `LED_COUNT`, `LED_PIN`, `BUTTON_PIN`, `LED_BRIGHTNESS` (0–255), `FPS`, and named LED strips/rings/matrices/groups |
| `helmets/<name>/States.h` | Animations, state transitions, button behavior, and initial state (`startHelmet()`) |
| `helmets/<name>/diagram.json` | Wokwi parts, LED lengths, and wiring |
| `include/animations/` | Shared effects that any helmet can use |

For example, a strip definition `using LeftVisor = Strip<0, 20>;` maps 20 LEDs starting at physical index 0. The next strip can start at 20. Keep the layout indices consistent with the **data-chain wiring order** in `diagram.json`.

**Rule of thumb:** If you're changing *where LEDs are*, edit `LEDLayouts.h`; if you're changing *what they do*, edit `States.h`. Don't edit shared `src/` files for helmet-specific behavior.

## Thomas visor controls

Thomas starts with a **static nine-color visor** (Purple, Cyan, Cyan, Green, Yellow, Yellow, Red, Red, Yellow), mapped as horizontal bands on each 4×10 visor. A **single press** cycles Glow → Chase → Charge (and from Party starts Glow). A **double press** enters Party and toggles Party Flow/Rush. A **long hold** returns to static visor from any state. Guy Manuel is unchanged.
