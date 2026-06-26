# ESP32 Physical Battery Emulator Controller

This sketch is for the second-ESP32 setup where one ESP32 runs the batView protocol/control side and another ESP32 emulates a battery electrically.

The ESP32 cannot act as a battery by itself. It must drive external hardware that can source and sink current safely.

## Expected Hardware

- Voltage divider from the emulator battery terminals into `PIN_VOLTAGE_SENSE`.
- Bidirectional current sensor into `PIN_CURRENT_SENSE`.
- Source stage driven by `GPIO32`, such as a controlled buck/boost supply.
- Sink stage driven by `GPIO33`, such as a MOSFET electronic load or bidirectional converter.
- C2000/C200 ADC input connected to `GPIO25` for scaled virtual battery voltage.
- Optional C2000/C200 ADC input connected to `GPIO26` for scaled current.
- Optional enable pin on `PIN_ENABLE`.

## Model

- Positive measured current means charging.
- Negative measured current means discharging.
- SOC is updated by coulomb counting.
- Open-circuit voltage comes from a 12 V battery SOC table.
- Terminal voltage target is:

```text
Vterminal = Voc + I * Rinternal
```

That means terminal voltage rises during charge and drops during discharge.

For this project model:

- Empty battery: `12.0 V`
- Full battery: `13.8 V`
- ESP32 analog output range: `0.0 V` to `3.3 V`
- C2000 voltage gain to recover battery voltage: `4.1818`

So:

```text
Vbattery = Vadc * 4.1818
Vadc = Vbattery / 4.1818
```

Expected ADC voltages:

```text
12.0 V battery -> 2.87 V at C2000 ADC
13.8 V battery -> 3.30 V at C2000 ADC
```

Current on `GPIO26` is centered:

```text
0.00 V = maximum discharge current
1.65 V = 0 A
3.30 V = maximum charge current
```

## Calibration

Before connecting to the control ESP32 power stage, adjust:

- `VOLTAGE_DIVIDER_RATIO`
- `CURRENT_ZERO_ADC_V`
- `CURRENT_SENSOR_V_PER_A`
- `CURRENT_SIGN`
- `MAX_CHARGE_CURRENT_A`
- `MAX_DISCHARGE_CURRENT_A`
- `KP` and `KI`

Start with current-limited supplies and low PWM duty limits while tuning.
