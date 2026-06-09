# PC <-> ESP32 Communication Protocol

batView communicates with the ESP32 over USB serial using newline-terminated text frames.

## Serial Settings

- Baud rate: `115200`
- Format: `8N1`
- Flow control: none
- Each frame starts with `#`
- Each frame is sent as one line, ending with `\n`

## PC to ESP32

The PC sends these commands:

```text
#CONNECTION
#Battery,<BatteryNameID>,<V@max>,<V@min>,<Amax>
#CICLE,0,0
#CICLE,1,<cycleCount>
#LOAD,<targetPercent>
#UNLOAD,<targetPercent>
#STOP
```

Notes:

- `BatteryNameID` must not contain commas.
- `V@max`, `V@min`, and `Amax` are decimal numbers.
- `V@min` must be lower than `V@max`.
- `targetPercent` is an integer from `0` to `100`.
- `#CICLE,0,0` means indefinite cycle mode.
- `#CICLE,1,<cycleCount>` means a fixed number of cycles.

## ESP32 to PC

For every command, the ESP32 should answer with the matching ACK:

```text
#ACK,CONNECTION
#ACK,Battery
#ACK,CICLE
#ACK,LOAD
#ACK,UNLOAD
#ACK,STOP
```

The app also accepts the legacy typo `#ACK,CONECTION` during connection.

The ESP32 may send status or error frames:

```text
#STATUS,<state>,<detail>
#ERROR,<code>,<message>
```

Telemetry is sent with `#DATA`:

```text
#DATA,<voltage>,<current>,<timestampMs>
#DATA,<voltage>,<current>,<timestampMs>,<state>,<completedCycles>
```

`timestampMs` must be milliseconds from the ESP32. The PC stores the received value and only converts it to seconds for plotting.

## Expected Flow

1. PC opens the serial port and sends `#CONNECTION`.
2. ESP32 answers `#ACK,CONNECTION`.
3. PC sends a battery profile with `#Battery,...`.
4. ESP32 answers `#ACK,Battery`.
5. PC sends `#LOAD,...`, `#UNLOAD,...`, or `#CICLE,...`.
6. ESP32 answers the matching ACK and starts sending `#DATA` frames.
7. PC may send `#STOP`.
8. ESP32 answers `#ACK,STOP` and stops the active operation.

If the ESP32 rejects a command, it should send `#ERROR,<code>,<message>` instead of the ACK.
