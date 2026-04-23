# PlatformIO ESP32 Upload

This is the PlatformIO version of the batView ESP32 test sketch.

Files:
- [platformio.ini](/Users/rodrigo/Documents/UCR/Proyecto_Electrico/proyecto_electrico/batView/examples/platformio/esp32_batview_test/platformio.ini)
- [src/main.cpp](/Users/rodrigo/Documents/UCR/Proyecto_Electrico/proyecto_electrico/batView/examples/platformio/esp32_batview_test/src/main.cpp)

## Current Port

The config is pre-filled with:

- `/dev/cu.usbserial-0001`

If your ESP32 appears on a different port, update `upload_port` and `monitor_port` in `platformio.ini`.

## Install PlatformIO Core

If you want the CLI version:

```bash
python3 -m pip install --user platformio
```

If `pio` still is not found after install, use:

```bash
python3 -m platformio --version
```

## Build

From this folder:

```bash
cd /Users/rodrigo/Documents/UCR/Proyecto_Electrico/proyecto_electrico/batView/examples/platformio/esp32_batview_test
python3 -m platformio run
```

## Upload

```bash
cd /Users/rodrigo/Documents/UCR/Proyecto_Electrico/proyecto_electrico/batView/examples/platformio/esp32_batview_test
python3 -m platformio run --target upload
```

## Serial Monitor

```bash
cd /Users/rodrigo/Documents/UCR/Proyecto_Electrico/proyecto_electrico/batView/examples/platformio/esp32_batview_test
python3 -m platformio device monitor
```

## If Upload Fails

- Confirm the correct board is selected in `platformio.ini`.
- If your board is not a generic ESP32 dev board, change:

```ini
board = esp32dev
```

to the correct PlatformIO board id for your hardware.

- Try pressing and holding `BOOT`, then run upload, and release `BOOT` when the upload starts.
- Make sure the cable supports data, not only power.
- Close any app that may already be using the serial port.

## Test Missing ACK

Edit the booleans at the top of `src/main.cpp`:

- `SEND_ACK_CONNECTION = false`
- `SEND_ACK_DATA = false`
- `SEND_ACK_LOAD = false`
- `SEND_ACK_UNLOAD = false`
- `SEND_ACK_CICLE = false`

Then rebuild and upload again.
