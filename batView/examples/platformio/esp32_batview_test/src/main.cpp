#include <Arduino.h>

static const unsigned long kBaudRate = 115200;
static const unsigned long kTelemetryPeriodMs = 1000;

// Toggle these to verify batView error handling.
static const bool SEND_ACK_CONNECTION = true;
static const bool SEND_ACK_DATA = true;
static const bool SEND_ACK_CICLE = true;
static const bool SEND_ACK_LOAD = true;
static const bool SEND_ACK_UNLOAD = true;
static const bool SEND_ACK_STOP = true;

static const bool SEND_ERROR_ON_DATA = false;
static const bool SEND_ERROR_ON_CICLE = false;
static const bool SEND_ERROR_ON_LOAD = false;
static const bool SEND_ERROR_ON_UNLOAD = false;
static const bool SEND_ERROR_ON_STOP = false;

String inputLine;
bool loadActive = false;
bool unloadActive = false;
bool cycleActive = false;
bool cycleInfinite = false;
int cycleCount = 0;
int completedCycles = 0;
unsigned long lastTelemetryAt = 0;
unsigned long startedAt = 0;

void processLine(String line);
void parseCycleMode(const String& line);
void streamTelemetryIfNeeded();
void sendShortTelemetry(float voltage, float current, unsigned long timestampMs);
void sendCycleTelemetry(float voltage, float current, unsigned long timestampMs, int state, int cycles);

void setup() {
  Serial.begin(kBaudRate);
  delay(500);

  startedAt = millis();
  Serial.println("#STATUS,BOOT,ESP32_READY");
}

void loop() {
  while (Serial.available() > 0) {
    const char ch = static_cast<char>(Serial.read());
    if (ch == '\r') {
      continue;
    }

    if (ch == '\n') {
      processLine(inputLine);
      inputLine = "";
      continue;
    }

    inputLine += ch;
  }

  streamTelemetryIfNeeded();
}

void processLine(String line) {
  line.trim();
  if (line.length() == 0) {
    return;
  }

  if (line == "#CONNECTION") {
    if (SEND_ACK_CONNECTION) {
      Serial.println("#ACK,CONNECTION");
      Serial.println("#STATUS,CONNECTED,ESP32_READY");
    }
    return;
  }

  if (line.startsWith("#DATA,")) {
    if (SEND_ERROR_ON_DATA) {
      Serial.println("#ERROR,11,BATTERY_SELECTION_REJECTED");
      return;
    }

    if (SEND_ACK_DATA) {
      Serial.println("#ACK,DATA");
      Serial.println("#STATUS,READY,BATTERY_SELECTED");
    }
    return;
  }

  if (line.startsWith("#CICLE,")) {
    if (SEND_ERROR_ON_CICLE) {
      Serial.println("#ERROR,21,CYCLE_CONFIGURATION_REJECTED");
      return;
    }

    parseCycleMode(line);
    loadActive = false;
    unloadActive = false;
    cycleActive = true;

    if (SEND_ACK_CICLE) {
      Serial.println("#ACK,CICLE");
      Serial.println("#STATUS,CYCLING,PROCESS_ACTIVE");
    }
    return;
  }

  if (line.startsWith("#LOAD,")) {
    if (SEND_ERROR_ON_LOAD) {
      Serial.println("#ERROR,31,LOAD_COMMAND_REJECTED");
      return;
    }

    unloadActive = false;
    cycleActive = false;
    loadActive = true;

    if (SEND_ACK_LOAD) {
      Serial.println("#ACK,LOAD");
      Serial.println("#STATUS,CHARGING,PROCESS_ACTIVE");
    }
    return;
  }

  if (line.startsWith("#UNLOAD,")) {
    if (SEND_ERROR_ON_UNLOAD) {
      Serial.println("#ERROR,41,UNLOAD_COMMAND_REJECTED");
      return;
    }

    loadActive = false;
    cycleActive = false;
    unloadActive = true;

    if (SEND_ACK_UNLOAD) {
      Serial.println("#ACK,UNLOAD");
      Serial.println("#STATUS,DISCHARGING,PROCESS_ACTIVE");
    }
    return;
  }

  if (line == "#STOP") {
    if (SEND_ERROR_ON_STOP) {
      Serial.println("#ERROR,51,STOP_REJECTED");
      return;
    }

    loadActive = false;
    unloadActive = false;
    cycleActive = false;

    if (SEND_ACK_STOP) {
      Serial.println("#ACK,STOP");
      Serial.println("#STATUS,STOPPED,PROCESS_IDLE");
    }
    return;
  }

  Serial.println("#ERROR,99,UNKNOWN_COMMAND");
}

void parseCycleMode(const String& line) {
  const int firstComma = line.indexOf(',');
  const int secondComma = line.indexOf(',', firstComma + 1);
  if (firstComma < 0 || secondComma < 0) {
    cycleInfinite = true;
    cycleCount = 0;
    return;
  }

  const String mode = line.substring(firstComma + 1, secondComma);
  const String count = line.substring(secondComma + 1);

  cycleInfinite = (mode == "0");
  cycleCount = count.toInt();
  completedCycles = 0;
}

void streamTelemetryIfNeeded() {
  if (!loadActive && !unloadActive && !cycleActive) {
    return;
  }

  const unsigned long now = millis();
  if (now - lastTelemetryAt < kTelemetryPeriodMs) {
    return;
  }

  lastTelemetryAt = now;

  if (loadActive) {
    const float voltage = 3.70f + (now % 40) * 0.01f;
    const float current = 1.00f + (now % 10) * 0.01f;
    sendShortTelemetry(voltage, current, now - startedAt);
    return;
  }

  if (unloadActive) {
    const float voltage = 4.20f - (now % 40) * 0.01f;
    const float current = 0.85f + (now % 10) * 0.01f;
    sendShortTelemetry(voltage, current, now - startedAt);
    return;
  }

  if (cycleActive) {
    if (!cycleInfinite && cycleCount > 0 && completedCycles < cycleCount) {
      completedCycles++;
    }

    const int state = (completedCycles % 2 == 0) ? 0 : 1;
    const float voltage = 3.80f + (completedCycles % 5) * 0.08f;
    const float current = 0.90f + (completedCycles % 3) * 0.05f;
    sendCycleTelemetry(voltage, current, now - startedAt, state, completedCycles);

    if (!cycleInfinite && cycleCount > 0 && completedCycles >= cycleCount) {
      cycleActive = false;
      Serial.println("#STATUS,FINISHED,CYCLE_COMPLETE");
    }
  }
}

void sendShortTelemetry(float voltage, float current, unsigned long timestampMs) {
  Serial.print("#DATA,");
  Serial.print(voltage, 2);
  Serial.print(",");
  Serial.print(current, 2);
  Serial.print(",");
  Serial.println(timestampMs);
}

void sendCycleTelemetry(float voltage, float current, unsigned long timestampMs, int state, int cycles) {
  Serial.print("#DATA,");
  Serial.print(voltage, 2);
  Serial.print(",");
  Serial.print(current, 2);
  Serial.print(",");
  Serial.print(timestampMs);
  Serial.print(",");
  Serial.print(state);
  Serial.print(",");
  Serial.println(cycles);
}
