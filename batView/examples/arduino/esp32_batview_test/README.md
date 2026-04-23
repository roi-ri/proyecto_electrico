# ESP32 batView Test Sketch

This folder contains a minimal Arduino IDE sketch for an ESP32 that speaks the serial protocol expected by batView.

Files:
- [esp32_batview_test.ino](/Users/rodrigo/Documents/UCR/Proyecto_Electrico/proyecto_electrico/batView/examples/arduino/esp32_batview_test/esp32_batview_test.ino)

## Upload Steps

1. Open Arduino IDE.
2. Open `examples/arduino/esp32_batview_test/esp32_batview_test.ino`.
3. Select your ESP32 board.
4. Select the correct serial port.
5. Upload the sketch.
6. Open batView and connect to the same port at `115200`.

## What batView Should See

- On connect, batView sends `#CONNECTION`.
- The ESP32 replies with `#ACK,CONNECTION`.
- After `#DATA,...`, `#LOAD,...`, `#UNLOAD,...`, `#CICLE,...`, and `#STOP`, the ESP32 replies with the expected `#ACK,...`.
- While `LOAD`, `UNLOAD`, or `CICLE` is active, batView should receive periodic `#DATA,...` frames.

## How To Test Missing ACK

Edit the booleans at the top of the sketch:

- Set `SEND_ACK_CONNECTION = false` to test the explicit connection timeout message.
- Set `SEND_ACK_DATA = false` to test battery selection without ACK.
- Set `SEND_ACK_LOAD = false` to test missing `#ACK,LOAD`.
- Set `SEND_ACK_UNLOAD = false` to test missing `#ACK,UNLOAD`.
- Set `SEND_ACK_CICLE = false` to test missing `#ACK,CICLE`.

Then upload again and retry from batView.

## How To Test ESP32 Error Frames

Set one of these to `true`:

- `SEND_ERROR_ON_DATA`
- `SEND_ERROR_ON_LOAD`
- `SEND_ERROR_ON_UNLOAD`
- `SEND_ERROR_ON_CICLE`
- `SEND_ERROR_ON_STOP`

batView should display the explicit `#ERROR,...` reason returned by the sketch.
