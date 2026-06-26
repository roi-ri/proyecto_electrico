# ESP-IDF Physical Battery Emulator

This is the `.c` / ESP-IDF version of the physical battery emulator controller.

## Pin Map

```text
GPIO25 / DAC1 -> C2000 ADC, scaled virtual battery voltage
GPIO26 / DAC2 -> C2000 ADC, optional scaled current
GPIO34 / ADC1 -> physical terminal voltage measurement
GPIO35 / ADC1 -> physical terminal current measurement
GPIO32 / PWM  -> external source-stage control
GPIO33 / PWM  -> external sink-stage control
GPIO27        -> optional enable
```

## C2000 Gain

Use this in the C2000:

```c
Vbattery = Vadc * 4.1818f;
```

Expected mapping:

```text
12.0 V battery -> 2.87 V ADC
13.8 V battery -> 3.30 V ADC
```

## Build

From this folder:

```sh
idf.py set-target esp32
idf.py build
idf.py flash monitor
```

Calibrate the constants at the top of `main/esp32_physical_battery_emulator.c` before connecting to higher-power hardware.
