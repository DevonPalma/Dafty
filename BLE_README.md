# Guy Manuel BLE messaging

This project is based on the supplied `Source.zip`. Thomas is unchanged. Only the
`guy_manuel_ble` environment enables Bluetooth and the scrolling marquee.

## Build / flash

The included BLE environment assumes **Arduino Nano 33 BLE**:

```sh
pio run -e guy_manuel_ble
pio run -e guy_manuel_ble -t upload
```

If your actual BLE board differs, update `platform` and `board` in
`platformio.ini`. The non-BLE `guy_manuel` and `thomas` environments still target
Uno R3; they have no BLE/GFX dependency.

## Phone

In a BLE app (nRF Connect / LightBlue), connect to **Dafty_Marquee** and write
UTF-8/ASCII bytes to:

- Service: `19B10000-E8F2-537E-4F6C-D104768A1214`
- Write characteristic: `19B10001-E8F2-537E-4F6C-D104768A1214`

The receiver stores up to 63 printable **ASCII** characters (not Unicode) and
scrolls them across the 48 x 8 matrix in cyan at a fixed 75 ms per pixel column (independent of FPS).
New messages restart the scroll at the right edge. The message starts fully offscreen and returns to Ambient only after the last character disappears off the left edge.
Single click dismisses text, double click enters Party, long hold returns
Ambient. Guy Manuel now uses only the 384-pixel matrix: no rings or ring effects. Thomas is unchanged.

## Implementation

- `lib/DaftyBluetooth/src/BluetoothText.h`: BLE receiver, fixed-size text buffer, no `String`.
- `helmets/guy_manuel/animations/TextAnimations.h`: GFX's built-in 5x7 font rendered through
  `FaceMatrix::setXY`, using the existing `pixels` buffer.
- `src/main.cpp`: BLE init and nonblocking polling under `DAFTY_ENABLE_BLE`.
- `helmets/guy_manuel/States.h`: only this helmet defines `Message` state and
  `helmetOnTextMessage()`.

BLE is not simulated by the supplied Wokwi Uno diagrams. For Nano 33 BLE hardware,
NeoPixel data output is **3.3V**: use an appropriate level shifter for a 5V WS2812B
chain, a properly rated external 5V LED supply, and common grounds.

**Not physically tested**: confirm the exact BLE board model before relying on
`nano33ble`, and test the actual build/upload on your machine.

## Wiring update

Guy Manuel now has **384 pixels**. Matrix DIN is wired directly to pin 6, with 5V and ground connections in the Wokwi diagram; the rings are removed from both diagram and firmware. For actual hardware use an external, suitably rated LED power supply with common ground.

Guy Manuel extra commands: `*Static HELLO` displays fixed text for 2.2s; `^heart_eyes` draws two animated pink hearts for 6s. See `helmets/guy_manuel/COMMANDS_README.md`.
