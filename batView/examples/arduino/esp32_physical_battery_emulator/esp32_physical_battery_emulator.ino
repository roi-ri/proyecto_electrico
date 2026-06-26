/*
  ESP32 physical battery-emulator controller.

  This is for the "second ESP32" idea:
  - ESP32 #1 runs the batView communication/control firmware.
  - ESP32 #2 behaves like the battery model.
  - ESP32 #2 measures the physical terminal voltage/current and drives
    external power electronics so the terminals look battery-like.

  Important:
  An ESP32 by itself cannot electrically behave as a battery. It cannot safely
  source or sink battery current from GPIO pins. Use external hardware:
  - Terminal voltage sensor through a resistor divider.
  - Bidirectional current sensor, for example ACS712/ACS758/INA module.
  - A source stage, for example DAC/PWM-controlled buck/boost supply.
  - A sink stage, for example MOSFET electronic load or bidirectional converter.
  - Optional analog outputs to a C2000/C200 ADC for scaled battery voltage/current.

  Current convention in this sketch:
  - Positive current means current entering the emulator terminals: charging.
  - Negative current means current leaving the emulator terminals: discharging.
*/

// ------------------------- Pin configuration -------------------------

static const int PIN_VOLTAGE_SENSE = 34;  // ADC1 input, terminal voltage divider.
static const int PIN_CURRENT_SENSE = 35;  // ADC1 input, bidirectional current sensor.
static const int PIN_SOURCE_PWM = 32;     // Drives source stage control input.
static const int PIN_SINK_PWM = 33;       // Drives sink/electronic-load control input.
static const int PIN_ENABLE = 27;         // Optional hardware enable.
static const int PIN_C2000_VBAT_DAC = 25; // DAC1: scaled virtual battery voltage.
static const int PIN_C2000_IBAT_DAC = 26; // DAC2: scaled virtual battery current.

// ------------------------- Battery model -----------------------------

static const float BATTERY_CAPACITY_AH = 2.20f;
static const float V_EMPTY = 12.00f;
static const float V_FULL = 13.80f;
static const float INTERNAL_RESISTANCE_OHM = 0.180f;

static const float MAX_CHARGE_CURRENT_A = 1.20f;
static const float MAX_DISCHARGE_CURRENT_A = 1.50f;
static const float MIN_SOC_PERCENT = 0.0f;
static const float MAX_SOC_PERCENT = 100.0f;

// C2000/C200 ADC scaling:
// ESP32 DAC output = virtual battery voltage / C2000_VBAT_GAIN.
// Use this same gain in the C2000 to recover the modeled battery voltage.
static const float C2000_ADC_MAX_V = 3.30f;
static const float C2000_VBAT_GAIN = V_FULL / C2000_ADC_MAX_V; // 13.8 / 3.3 = 4.1818
static const float C2000_CURRENT_FULL_SCALE_A = 2.00f;

// Change this to start the virtual battery fuller or emptier.
float socPercent = 55.0f;

// ------------------------- Sensor calibration ------------------------

// ESP32 ADC nominal reference. Real boards vary; calibrate with a multimeter.
static const float ADC_REFERENCE_V = 3.30f;
static const int ADC_MAX = 4095;

// Voltage divider: terminal voltage -> ESP32 ADC.
// Example: R_TOP = 100k, R_BOTTOM = 33k gives ratio around 4.03.
static const float VOLTAGE_DIVIDER_RATIO = 4.03f;

// Current sensor analog calibration.
// Many bidirectional sensors sit near Vcc/2 at 0 A.
static const float CURRENT_ZERO_ADC_V = 1.65f;
static const float CURRENT_SENSOR_V_PER_A = 0.185f;  // ACS712 5A is about 185 mV/A.

// Set to -1.0 if your sensor sign is backwards.
static const float CURRENT_SIGN = 1.0f;

// ------------------------- Control configuration ---------------------

static const unsigned long CONTROL_PERIOD_MS = 20;
static const unsigned long LOG_PERIOD_MS = 500;

static const int PWM_FREQ_HZ = 20000;
static const int PWM_RESOLUTION_BITS = 10;
static const int PWM_MAX_DUTY = (1 << PWM_RESOLUTION_BITS) - 1;
static const int PWM_SOURCE_CHANNEL = 0;
static const int PWM_SINK_CHANNEL = 1;

// PI gains for a simple terminal-voltage controller.
// Start low, then tune with current limits and an oscilloscope/multimeter.
static const float KP = 0.85f;
static const float KI = 0.30f;
static const float INTEGRATOR_LIMIT = 1.0f;

float integrator = 0.0f;
unsigned long lastControlAt = 0;
unsigned long lastLogAt = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_VOLTAGE_SENSE, ADC_11db);
  analogSetPinAttenuation(PIN_CURRENT_SENSE, ADC_11db);

  ledcSetup(PWM_SOURCE_CHANNEL, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
  ledcSetup(PWM_SINK_CHANNEL, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
  ledcAttachPin(PIN_SOURCE_PWM, PWM_SOURCE_CHANNEL);
  ledcAttachPin(PIN_SINK_PWM, PWM_SINK_CHANNEL);

  pinMode(PIN_ENABLE, OUTPUT);
  digitalWrite(PIN_ENABLE, HIGH);

  setActuators(0.0f, 0.0f);
  lastControlAt = millis();

  Serial.println("BATTERY_EMULATOR_READY");
  Serial.println("CSV:ms,soc,ocv_v,target_v,terminal_v,current_a,source_pwm,sink_pwm");
}

void loop() {
  const unsigned long now = millis();

  if (now - lastControlAt >= CONTROL_PERIOD_MS) {
    const float dtSeconds = (now - lastControlAt) / 1000.0f;
    lastControlAt = now;
    runControlStep(now, dtSeconds);
  }
}

void runControlStep(unsigned long now, float dtSeconds) {
  const float terminalVoltage = readTerminalVoltage();
  const float terminalCurrent = readTerminalCurrent();

  updateSocByCoulombCounting(terminalCurrent, dtSeconds);

  const float ocv = openCircuitVoltageFromSoc(socPercent);
  const float targetVoltage = batteryTerminalTargetVoltage(ocv, terminalCurrent);

  float sourceCommand = 0.0f;
  float sinkCommand = 0.0f;

  if (socPercent <= MIN_SOC_PERCENT && terminalCurrent < 0.0f) {
    // Empty battery: stop delivering current.
    sourceCommand = 0.0f;
    sinkCommand = 0.0f;
    integrator = 0.0f;
  } else if (socPercent >= MAX_SOC_PERCENT && terminalCurrent > 0.0f) {
    // Full battery: stop accepting charge current.
    sourceCommand = 0.0f;
    sinkCommand = 0.0f;
    integrator = 0.0f;
  } else {
    const float error = targetVoltage - terminalVoltage;
    integrator += error * KI * dtSeconds;
    integrator = constrain(integrator, -INTEGRATOR_LIMIT, INTEGRATOR_LIMIT);

    const float command = constrain(error * KP + integrator, -1.0f, 1.0f);

    if (command >= 0.0f) {
      sourceCommand = command;
      sinkCommand = 0.0f;
    } else {
      sourceCommand = 0.0f;
      sinkCommand = -command;
    }
  }

  applyCurrentProtection(terminalCurrent, sourceCommand, sinkCommand);
  setActuators(sourceCommand, sinkCommand);
  writeC2000AnalogOutputs(targetVoltage, terminalCurrent);

  if (now - lastLogAt >= LOG_PERIOD_MS) {
    lastLogAt = now;
    logTelemetry(now, socPercent, ocv, targetVoltage, terminalVoltage, terminalCurrent,
                 sourceCommand, sinkCommand);
  }
}

void updateSocByCoulombCounting(float terminalCurrentA, float dtSeconds) {
  const float deltaAh = terminalCurrentA * dtSeconds / 3600.0f;
  const float deltaSoc = (deltaAh / BATTERY_CAPACITY_AH) * 100.0f;
  socPercent += deltaSoc;
  socPercent = constrain(socPercent, MIN_SOC_PERCENT, MAX_SOC_PERCENT);
}

float openCircuitVoltageFromSoc(float soc) {
  const float x = constrain(soc / 100.0f, 0.0f, 1.0f);

  // Simple Li-ion-like OCV curve. Replace this table if your project uses
  // another battery chemistry.
  static const float socTable[] = {0.00f, 0.10f, 0.20f, 0.40f, 0.60f, 0.80f, 0.90f, 1.00f};
  static const float voltTable[] = {12.00f, 12.15f, 12.35f, 12.60f, 12.85f, 13.15f, 13.45f, 13.80f};
  static const int tableSize = sizeof(socTable) / sizeof(socTable[0]);

  if (x <= socTable[0]) {
    return voltTable[0];
  }

  for (int i = 1; i < tableSize; ++i) {
    if (x <= socTable[i]) {
      const float span = socTable[i] - socTable[i - 1];
      const float t = (x - socTable[i - 1]) / span;
      return voltTable[i - 1] + t * (voltTable[i] - voltTable[i - 1]);
    }
  }

  return voltTable[tableSize - 1];
}

float batteryTerminalTargetVoltage(float ocv, float terminalCurrentA) {
  // Positive current charges the battery, so terminal voltage rises above OCV.
  // Negative current discharges the battery, so terminal voltage falls below OCV.
  const float target = ocv + terminalCurrentA * INTERNAL_RESISTANCE_OHM;
  return constrain(target, V_EMPTY, V_FULL);
}

float readTerminalVoltage() {
  const int raw = analogRead(PIN_VOLTAGE_SENSE);
  const float adcVoltage = rawToAdcVoltage(raw);
  return adcVoltage * VOLTAGE_DIVIDER_RATIO;
}

float readTerminalCurrent() {
  const int raw = analogRead(PIN_CURRENT_SENSE);
  const float adcVoltage = rawToAdcVoltage(raw);
  return CURRENT_SIGN * ((adcVoltage - CURRENT_ZERO_ADC_V) / CURRENT_SENSOR_V_PER_A);
}

float rawToAdcVoltage(int raw) {
  return (static_cast<float>(raw) / ADC_MAX) * ADC_REFERENCE_V;
}

void applyCurrentProtection(float terminalCurrentA, float& sourceCommand, float& sinkCommand) {
  if (terminalCurrentA > MAX_CHARGE_CURRENT_A) {
    sinkCommand = 0.0f;
  }

  if (terminalCurrentA < -MAX_DISCHARGE_CURRENT_A) {
    sourceCommand = 0.0f;
  }
}

void setActuators(float sourceCommand, float sinkCommand) {
  sourceCommand = constrain(sourceCommand, 0.0f, 1.0f);
  sinkCommand = constrain(sinkCommand, 0.0f, 1.0f);

  ledcWrite(PWM_SOURCE_CHANNEL, static_cast<int>(sourceCommand * PWM_MAX_DUTY));
  ledcWrite(PWM_SINK_CHANNEL, static_cast<int>(sinkCommand * PWM_MAX_DUTY));
}

void writeC2000AnalogOutputs(float virtualBatteryVoltage, float terminalCurrentA) {
  const float scaledBatteryVoltage = constrain(
      virtualBatteryVoltage / C2000_VBAT_GAIN,
      0.0f,
      C2000_ADC_MAX_V);

  // Current output is centered at 1.65 V so it can represent charge/discharge:
  // 0.00 V = -full scale discharge, 1.65 V = 0 A, 3.30 V = +full scale charge.
  const float scaledCurrentVoltage = constrain(
      (C2000_ADC_MAX_V * 0.5f) +
          (terminalCurrentA / C2000_CURRENT_FULL_SCALE_A) * (C2000_ADC_MAX_V * 0.5f),
      0.0f,
      C2000_ADC_MAX_V);

  dacWrite(PIN_C2000_VBAT_DAC, voltageToDacCode(scaledBatteryVoltage));
  dacWrite(PIN_C2000_IBAT_DAC, voltageToDacCode(scaledCurrentVoltage));
}

int voltageToDacCode(float voltage) {
  return static_cast<int>(constrain((voltage / C2000_ADC_MAX_V) * 255.0f, 0.0f, 255.0f));
}

void logTelemetry(unsigned long now,
                  float soc,
                  float ocv,
                  float targetVoltage,
                  float terminalVoltage,
                  float terminalCurrent,
                  float sourceCommand,
                  float sinkCommand) {
  Serial.print(now);
  Serial.print(",");
  Serial.print(soc, 2);
  Serial.print(",");
  Serial.print(ocv, 3);
  Serial.print(",");
  Serial.print(targetVoltage, 3);
  Serial.print(",");
  Serial.print(terminalVoltage, 3);
  Serial.print(",");
  Serial.print(terminalCurrent, 3);
  Serial.print(",");
  Serial.print(sourceCommand, 3);
  Serial.print(",");
  Serial.println(sinkCommand, 3);
}
