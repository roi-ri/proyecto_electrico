# ESP32 Battery Simulator for batView

This Arduino sketch makes an ESP32 behave like a simulated battery source for batView over USB serial.

## Upload

1. Open `esp32_battery_simulator.ino` in Arduino IDE.
2. Select your ESP32 board and USB serial port.
3. Upload the sketch.
4. Open batView and connect to the ESP32 serial port at `115200`.

## Behavior

- Answers `#CONNECTION` with `#ACK,CONNECTION`.
- Accepts the selected battery profile from batView.
- Simulates charge with `#LOAD,<targetPercent>`.
- Simulates discharge with `#UNLOAD,<targetPercent>`.
- Simulates repeated charge/discharge cycles with `#CICLE,...`.
- Streams telemetry as `#DATA,<voltage>,<current>,<timestampMs>,<state>,<completedCycles>`.

Current is positive while charging and negative while discharging.
