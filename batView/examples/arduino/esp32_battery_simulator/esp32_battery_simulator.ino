/*
  ESP32 battery simulator for batView.

  Upload this sketch to an ESP32, connect it to the computer by USB, then
  select the ESP32 serial port in batView at 115200 baud.

  The sketch does not control real battery hardware. It only simulates the
  serial protocol and telemetry that batView expects.
*/

static const unsigned long kBaudRate = 115200;
static const unsigned long kTelemetryPeriodMs = 1000;

enum Operation {
  OP_IDLE,
  OP_CHARGE,
  OP_DISCHARGE,
  OP_CYCLE_CHARGE,
  OP_CYCLE_DISCHARGE
};

String inputLine;
Operation operation = OP_IDLE;

String batteryName = "SIM_PACK";
float maxVoltage = 4.20f;
float minVoltage = 3.00f;
float maxCurrent = 1.50f;

float socPercent = 55.0f;
int targetPercent = 100;
bool cycleInfinite = true;
int requestedCycles = 0;
int completedCycles = 0;

unsigned long bootAt = 0;
unsigned long lastTelemetryAt = 0;
unsigned long lastSimulationAt = 0;

void setup() {
  Serial.begin(kBaudRate);
  delay(500);

  bootAt = millis();
  lastSimulationAt = bootAt;

  Serial.println("#STATUS,BOOT,ESP32_BATTERY_SIM_READY");
}

void loop() {
  readSerialCommands();
  updateBatterySimulation();
  sendTelemetryIfActive();
}

void readSerialCommands() {
  while (Serial.available() > 0) {
    const char ch = static_cast<char>(Serial.read());

    if (ch == '\r') {
      continue;
    }

    if (ch == '\n') {
      inputLine.trim();
      if (inputLine.length() > 0) {
        processCommand(inputLine);
      }
      inputLine = "";
      continue;
    }

    inputLine += ch;
  }
}

void processCommand(const String& line) {
  if (line == "#CONNECTION") {
    Serial.println("#ACK,CONNECTION");
    Serial.println("#STATUS,CONNECTED,ESP32_BATTERY_SIM_READY");
    return;
  }

  if (line.startsWith("#Battery,")) {
    handleBatteryProfile(line);
    return;
  }

  if (line.startsWith("#LOAD,")) {
    const int requestedTarget = line.substring(6).toInt();
    if (!isValidPercent(requestedTarget)) {
      Serial.println("#ERROR,31,LOAD_TARGET_OUT_OF_RANGE");
      return;
    }

    targetPercent = requestedTarget;
    operation = OP_CHARGE;
    resetTelemetryTimer();

    Serial.println("#ACK,LOAD");
    Serial.println("#STATUS,CHARGING,PROCESS_ACTIVE");
    return;
  }

  if (line.startsWith("#UNLOAD,")) {
    const int requestedTarget = line.substring(8).toInt();
    if (!isValidPercent(requestedTarget)) {
      Serial.println("#ERROR,41,UNLOAD_TARGET_OUT_OF_RANGE");
      return;
    }

    targetPercent = requestedTarget;
    operation = OP_DISCHARGE;
    resetTelemetryTimer();

    Serial.println("#ACK,UNLOAD");
    Serial.println("#STATUS,DISCHARGING,PROCESS_ACTIVE");
    return;
  }

  if (line.startsWith("#CICLE,")) {
    handleCycleCommand(line);
    return;
  }

  if (line == "#STOP") {
    operation = OP_IDLE;

    Serial.println("#ACK,STOP");
    Serial.println("#STATUS,STOPPED,PROCESS_IDLE");
    return;
  }

  Serial.println("#ERROR,99,UNKNOWN_COMMAND");
}

void handleBatteryProfile(const String& line) {
  const int firstComma = line.indexOf(',');
  const int secondComma = line.indexOf(',', firstComma + 1);
  const int thirdComma = line.indexOf(',', secondComma + 1);
  const int fourthComma = line.indexOf(',', thirdComma + 1);

  if (firstComma < 0 || secondComma < 0 || thirdComma < 0 || fourthComma < 0) {
    Serial.println("#ERROR,11,BATTERY_PROFILE_FORMAT");
    return;
  }

  const String name = line.substring(firstComma + 1, secondComma);
  const float parsedMaxVoltage = line.substring(secondComma + 1, thirdComma).toFloat();
  const float parsedMinVoltage = line.substring(thirdComma + 1, fourthComma).toFloat();
  const float parsedMaxCurrent = line.substring(fourthComma + 1).toFloat();

  if (name.length() == 0 || parsedMaxVoltage <= 0.0f || parsedMinVoltage <= 0.0f ||
      parsedMinVoltage >= parsedMaxVoltage || parsedMaxCurrent <= 0.0f) {
    Serial.println("#ERROR,11,BATTERY_PROFILE_REJECTED");
    return;
  }

  batteryName = name;
  maxVoltage = parsedMaxVoltage;
  minVoltage = parsedMinVoltage;
  maxCurrent = parsedMaxCurrent;
  socPercent = constrain(socPercent, 0.0f, 100.0f);

  Serial.println("#ACK,Battery");
  Serial.println("#STATUS,READY,BATTERY_PROFILE_SELECTED");
}

void handleCycleCommand(const String& line) {
  const int firstComma = line.indexOf(',');
  const int secondComma = line.indexOf(',', firstComma + 1);

  if (firstComma < 0 || secondComma < 0) {
    Serial.println("#ERROR,21,CYCLE_CONFIGURATION_FORMAT");
    return;
  }

  const int mode = line.substring(firstComma + 1, secondComma).toInt();
  const int count = line.substring(secondComma + 1).toInt();

  if (mode != 0 && mode != 1) {
    Serial.println("#ERROR,21,CYCLE_MODE_REJECTED");
    return;
  }

  if (mode == 1 && count <= 0) {
    Serial.println("#ERROR,21,CYCLE_COUNT_REJECTED");
    return;
  }

  cycleInfinite = (mode == 0);
  requestedCycles = count;
  completedCycles = 0;
  operation = OP_CYCLE_DISCHARGE;
  resetTelemetryTimer();

  Serial.println("#ACK,CICLE");
  Serial.println("#STATUS,CYCLING,PROCESS_ACTIVE");
}

void updateBatterySimulation() {
  const unsigned long now = millis();
  const float elapsedSeconds = (now - lastSimulationAt) / 1000.0f;
  if (elapsedSeconds <= 0.0f) {
    return;
  }

  lastSimulationAt = now;

  switch (operation) {
    case OP_CHARGE:
      socPercent += 2.0f * elapsedSeconds;
      if (socPercent >= targetPercent) {
        socPercent = targetPercent;
        operation = OP_IDLE;
        Serial.println("#STATUS,FINISHED,LOAD_TARGET_REACHED");
      }
      break;

    case OP_DISCHARGE:
      socPercent -= 2.0f * elapsedSeconds;
      if (socPercent <= targetPercent) {
        socPercent = targetPercent;
        operation = OP_IDLE;
        Serial.println("#STATUS,FINISHED,UNLOAD_TARGET_REACHED");
      }
      break;

    case OP_CYCLE_CHARGE:
      socPercent += 2.5f * elapsedSeconds;
      if (socPercent >= 90.0f) {
        socPercent = 90.0f;
        completedCycles++;
        if (!cycleInfinite && completedCycles >= requestedCycles) {
          operation = OP_IDLE;
          Serial.println("#STATUS,FINISHED,CYCLE_COMPLETE");
        } else {
          operation = OP_CYCLE_DISCHARGE;
          Serial.println("#STATUS,CYCLING,DISCHARGE_STEP");
        }
      }
      break;

    case OP_CYCLE_DISCHARGE:
      socPercent -= 2.5f * elapsedSeconds;
      if (socPercent <= 20.0f) {
        socPercent = 20.0f;
        operation = OP_CYCLE_CHARGE;
        Serial.println("#STATUS,CYCLING,CHARGE_STEP");
      }
      break;

    case OP_IDLE:
    default:
      break;
  }

  socPercent = constrain(socPercent, 0.0f, 100.0f);
}

void sendTelemetryIfActive() {
  if (operation == OP_IDLE) {
    return;
  }

  const unsigned long now = millis();
  if (now - lastTelemetryAt < kTelemetryPeriodMs) {
    return;
  }

  lastTelemetryAt = now;

  const float voltage = simulatedVoltage(now);
  const float current = simulatedCurrent(now);
  const int state = simulatedStateCode();

  Serial.print("#DATA,");
  Serial.print(voltage, 2);
  Serial.print(",");
  Serial.print(current, 2);
  Serial.print(",");
  Serial.print(now - bootAt);
  Serial.print(",");
  Serial.print(state);
  Serial.print(",");
  Serial.println(completedCycles);
}

float simulatedVoltage(unsigned long now) {
  const float normalizedSoc = socPercent / 100.0f;
  const float ripple = ((now / 250) % 5) * 0.005f;
  return minVoltage + (maxVoltage - minVoltage) * normalizedSoc + ripple;
}

float simulatedCurrent(unsigned long now) {
  const float ripple = ((now / 200) % 6) * 0.01f;
  const float baseCurrent = maxCurrent * 0.65f + ripple;

  if (operation == OP_CHARGE || operation == OP_CYCLE_CHARGE) {
    return baseCurrent;
  }

  if (operation == OP_DISCHARGE || operation == OP_CYCLE_DISCHARGE) {
    return -baseCurrent;
  }

  return 0.0f;
}

int simulatedStateCode() {
  if (operation == OP_CHARGE || operation == OP_CYCLE_CHARGE) {
    return 1;
  }

  if (operation == OP_DISCHARGE || operation == OP_CYCLE_DISCHARGE) {
    return 0;
  }

  return 2;
}

bool isValidPercent(int value) {
  return value >= 0 && value <= 100;
}

void resetTelemetryTimer() {
  lastTelemetryAt = 0;
  lastSimulationAt = millis();
}
