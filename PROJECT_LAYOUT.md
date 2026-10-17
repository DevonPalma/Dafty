# Super Dafty source layout

- `src/main.cpp`: shared firmware entry point.
- `include/`: shared header-only LED geometry, colors, state engine, and the animation suite (`include/animations/`).
- `lib/DaftyButton/src/Button.h`: reusable button input driver.
- `lib/DaftyBluetooth/src/BluetoothText.h`: reusable BLE message transport, compiled only by BLE-enabled builds.
- `helmets/<helmet>/LEDLayouts.h`: pins, LED order, brightness, frame rate.
- `helmets/<helmet>/States.h`: each helmet's independent state machine behavior.
- `helmets/guy_manuel/commands/`: Guy's parser and text configuration; no BLE transport logic.
- `helmets/guy_manuel/animations/`: Guy's text and specialized effects.

## Build / flash

```sh
pio run -e thomas
pio run -e guy_manuel
pio run -e guy_manuel_ble
pio run -e guy_manuel_ble -t upload
```

`guy_manuel_ble` currently targets a Nano 33 BLE **as an example**; change its board target to match your physical BLE board before uploading.
For Wokwi, build the selected environment and select the helmet's `wokwi.toml` in the Wokwi VS Code extension.

Only `guy_manuel_ble` includes Bluetooth/Adafruit GFX. Thomas and Guy's plain AVR build do not.
