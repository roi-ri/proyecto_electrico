/*
 * ESP32 physical battery-emulator controller for ESP-IDF.
 *
 * Battery model:
 * - Empty battery: 12.0 V
 * - Full battery: 13.8 V
 * - C2000/C200 voltage ADC gain: 4.1818
 *
 * Pin map, ESP32 classic:
 * - GPIO25 / DAC1: scaled virtual battery voltage to C2000 ADC.
 * - GPIO26 / DAC2: optional scaled current output to C2000 ADC.
 * - GPIO34 / ADC1_CH6: measured physical terminal voltage.
 * - GPIO35 / ADC1_CH7: measured physical terminal current.
 * - GPIO32 / LEDC PWM: source-stage command.
 * - GPIO33 / LEDC PWM: sink-stage command.
 * - GPIO27: optional hardware enable.
 *
 * Current convention:
 * - Positive current means current entering the emulator: charging.
 * - Negative current means current leaving the emulator: discharging.
 *
 * Important:
 * The ESP32 cannot source/sink battery current directly. GPIO25/GPIO26 are
 * only low-power analog signals for the C2000 ADC. GPIO32/GPIO33 should drive
 * external power electronics through proper gate drivers/control inputs.
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include "driver/adc.h"
#include "driver/dac.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc_cal.h"
#include "esp_check.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "battery_emulator"

#define PIN_ENABLE GPIO_NUM_27
#define PIN_SOURCE_PWM GPIO_NUM_32
#define PIN_SINK_PWM GPIO_NUM_33

#define ADC_VOLTAGE_CHANNEL ADC1_CHANNEL_6 /* GPIO34 */
#define ADC_CURRENT_CHANNEL ADC1_CHANNEL_7 /* GPIO35 */

#define CONTROL_PERIOD_MS 20
#define LOG_PERIOD_MS 500

#define PWM_FREQ_HZ 20000
#define PWM_RESOLUTION LEDC_TIMER_10_BIT
#define PWM_MAX_DUTY ((1 << 10) - 1)
#define PWM_TIMER LEDC_TIMER_0
#define PWM_MODE LEDC_LOW_SPEED_MODE
#define PWM_SOURCE_CHANNEL LEDC_CHANNEL_0
#define PWM_SINK_CHANNEL LEDC_CHANNEL_1

static const float BATTERY_CAPACITY_AH = 2.20f;
static const float V_EMPTY = 12.00f;
static const float V_FULL = 13.80f;
static const float INTERNAL_RESISTANCE_OHM = 0.180f;

static const float MAX_CHARGE_CURRENT_A = 1.20f;
static const float MAX_DISCHARGE_CURRENT_A = 1.50f;
static const float MIN_SOC_PERCENT = 0.0f;
static const float MAX_SOC_PERCENT = 100.0f;

/* Example divider: Rtop = 100k, Rbottom = 33k, ratio approximately 4.03. */
static const float VOLTAGE_DIVIDER_RATIO = 4.03f;

/* Example ACS712-5A style current sensor. Calibrate for your sensor. */
static const float CURRENT_ZERO_ADC_V = 1.65f;
static const float CURRENT_SENSOR_V_PER_A = 0.185f;
static const float CURRENT_SIGN = 1.0f;

static const float C2000_ADC_MAX_V = 3.30f;
static const float C2000_VBAT_GAIN = 13.80f / 3.30f; /* 4.1818 */
static const float C2000_CURRENT_FULL_SCALE_A = 2.00f;

static const float KP = 0.85f;
static const float KI = 0.30f;
static const float INTEGRATOR_LIMIT = 1.0f;

static float soc_percent = 55.0f;
static float integrator = 0.0f;
static esp_adc_cal_characteristics_t adc_chars;

static float clampf(float value, float min_value, float max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static uint8_t voltage_to_dac_code(float voltage) {
    const float clamped = clampf(voltage, 0.0f, C2000_ADC_MAX_V);
    return (uint8_t)lroundf((clamped / C2000_ADC_MAX_V) * 255.0f);
}

static float raw_to_adc_voltage(int raw) {
    /*
     * ESP32 ADCs are not perfectly linear, especially with 11 dB attenuation.
     * esp_adc_cal_raw_to_voltage() returns millivolts using eFuse calibration
     * when available and the configured default Vref otherwise.
     */
    const uint32_t mv = esp_adc_cal_raw_to_voltage((uint32_t)raw, &adc_chars);
    return (float)mv / 1000.0f;
}

static float read_terminal_voltage(void) {
    const int raw = adc1_get_raw(ADC_VOLTAGE_CHANNEL);
    const float adc_voltage = raw_to_adc_voltage(raw);
    return adc_voltage * VOLTAGE_DIVIDER_RATIO;
}

static float read_terminal_current(void) {
    const int raw = adc1_get_raw(ADC_CURRENT_CHANNEL);
    const float adc_voltage = raw_to_adc_voltage(raw);
    return CURRENT_SIGN * ((adc_voltage - CURRENT_ZERO_ADC_V) / CURRENT_SENSOR_V_PER_A);
}

static float open_circuit_voltage_from_soc(float soc) {
    static const float soc_table[] = {
        0.00f, 0.10f, 0.20f, 0.40f, 0.60f, 0.80f, 0.90f, 1.00f
    };
    static const float volt_table[] = {
        12.00f, 12.15f, 12.35f, 12.60f, 12.85f, 13.15f, 13.45f, 13.80f
    };
    const int table_size = sizeof(soc_table) / sizeof(soc_table[0]);
    const float x = clampf(soc / 100.0f, 0.0f, 1.0f);

    if (x <= soc_table[0]) {
        return volt_table[0];
    }

    for (int i = 1; i < table_size; ++i) {
        if (x <= soc_table[i]) {
            const float span = soc_table[i] - soc_table[i - 1];
            const float t = (x - soc_table[i - 1]) / span;
            return volt_table[i - 1] + t * (volt_table[i] - volt_table[i - 1]);
        }
    }

    return volt_table[table_size - 1];
}

static float battery_terminal_target_voltage(float ocv, float terminal_current_a) {
    const float target = ocv + terminal_current_a * INTERNAL_RESISTANCE_OHM;
    return clampf(target, V_EMPTY, V_FULL);
}

static void update_soc_by_coulomb_counting(float terminal_current_a, float dt_seconds) {
    const float delta_ah = terminal_current_a * dt_seconds / 3600.0f;
    const float delta_soc = (delta_ah / BATTERY_CAPACITY_AH) * 100.0f;
    soc_percent = clampf(soc_percent + delta_soc, MIN_SOC_PERCENT, MAX_SOC_PERCENT);
}

static void apply_current_protection(float terminal_current_a,
                                     float *source_command,
                                     float *sink_command) {
    if (terminal_current_a > MAX_CHARGE_CURRENT_A) {
        /*
         * Charge overcurrent: stop accepting energy into the emulator.
         * In this model the sink stage is the charge-accept path.
         */
        *sink_command = 0.0f;
    }

    if (terminal_current_a < -MAX_DISCHARGE_CURRENT_A) {
        /*
         * Discharge overcurrent: stop delivering energy from the emulator.
         * In this model the source stage is the discharge-delivery path.
         */
        *source_command = 0.0f;
    }
}

static esp_err_t write_pwm(ledc_channel_t channel, float command) {
    const float clamped = clampf(command, 0.0f, 1.0f);
    const uint32_t duty = (uint32_t)lroundf(clamped * PWM_MAX_DUTY);

    ESP_RETURN_ON_ERROR(ledc_set_duty(PWM_MODE, channel, duty), TAG, "set duty failed");
    return ledc_update_duty(PWM_MODE, channel);
}

static void set_actuators(float source_command, float sink_command) {
    ESP_ERROR_CHECK(write_pwm(PWM_SOURCE_CHANNEL, source_command));
    ESP_ERROR_CHECK(write_pwm(PWM_SINK_CHANNEL, sink_command));
}

static void write_c2000_analog_outputs(float virtual_battery_voltage,
                                       float terminal_current_a) {
    const float scaled_battery_voltage =
        clampf(virtual_battery_voltage / C2000_VBAT_GAIN, 0.0f, C2000_ADC_MAX_V);

    /*
     * Current output is centered at 1.65 V:
     * - 0.00 V = max discharge
     * - 1.65 V = 0 A
     * - 3.30 V = max charge
     */
    const float scaled_current_voltage =
        clampf((C2000_ADC_MAX_V * 0.5f) +
                   (terminal_current_a / C2000_CURRENT_FULL_SCALE_A) *
                       (C2000_ADC_MAX_V * 0.5f),
               0.0f,
               C2000_ADC_MAX_V);

    ESP_ERROR_CHECK(dac_output_voltage(DAC_CHANNEL_1,
                                       voltage_to_dac_code(scaled_battery_voltage)));
    ESP_ERROR_CHECK(dac_output_voltage(DAC_CHANNEL_2,
                                       voltage_to_dac_code(scaled_current_voltage)));
}

static void log_telemetry(uint32_t now_ms,
                          float soc,
                          float ocv,
                          float target_voltage,
                          float terminal_voltage,
                          float terminal_current,
                          float source_command,
                          float sink_command) {
    printf("%" PRIu32 ",%.2f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
           now_ms,
           soc,
           ocv,
           target_voltage,
           terminal_voltage,
           terminal_current,
           source_command,
           sink_command);
}

static void configure_adc(void) {
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC_VOLTAGE_CHANNEL, ADC_ATTEN_DB_11));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC_CURRENT_CHANNEL, ADC_ATTEN_DB_11));

    /*
     * Default Vref is 1100 mV. If the module has ADC eFuse calibration,
     * ESP-IDF uses it automatically.
     */
    const esp_adc_cal_value_t cal_type =
        esp_adc_cal_characterize(ADC_UNIT_1,
                                 ADC_ATTEN_DB_11,
                                 ADC_WIDTH_BIT_12,
                                 1100,
                                 &adc_chars);

    if (cal_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("ADC cal: two-point from eFuse\n");
    } else if (cal_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("ADC cal: Vref from eFuse\n");
    } else {
        printf("ADC cal: default Vref 1100 mV; measure and adjust if needed\n");
    }
}

static void configure_dac(void) {
    ESP_ERROR_CHECK(dac_output_enable(DAC_CHANNEL_1)); /* GPIO25 */
    ESP_ERROR_CHECK(dac_output_enable(DAC_CHANNEL_2)); /* GPIO26 */
    write_c2000_analog_outputs(V_EMPTY, 0.0f);
}

static void configure_pwm(void) {
    const ledc_timer_config_t timer_config = {
        .speed_mode = PWM_MODE,
        .duty_resolution = PWM_RESOLUTION,
        .timer_num = PWM_TIMER,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    const ledc_channel_config_t source_config = {
        .gpio_num = PIN_SOURCE_PWM,
        .speed_mode = PWM_MODE,
        .channel = PWM_SOURCE_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = PWM_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&source_config));

    const ledc_channel_config_t sink_config = {
        .gpio_num = PIN_SINK_PWM,
        .speed_mode = PWM_MODE,
        .channel = PWM_SINK_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = PWM_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&sink_config));
}

static void configure_enable_pin(void) {
    const gpio_config_t enable_config = {
        .pin_bit_mask = 1ULL << PIN_ENABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&enable_config));
    ESP_ERROR_CHECK(gpio_set_level(PIN_ENABLE, 1));
}

static void battery_emulator_task(void *arg) {
    (void)arg;

    TickType_t last_wake = xTaskGetTickCount();
    TickType_t last_log_tick = last_wake;

    printf("BATTERY_EMULATOR_READY\n");
    printf("Use gain in C2000: Vbattery = Vadc * %.4f\n", C2000_VBAT_GAIN);
    printf("CSV:ms,soc,ocv_v,target_v,terminal_v,current_a,source_pwm,sink_pwm\n");

    while (true) {
        static const float dt_seconds = CONTROL_PERIOD_MS / 1000.0f;
        const TickType_t now_tick = xTaskGetTickCount();

        const float terminal_voltage = read_terminal_voltage();
        const float terminal_current = read_terminal_current();

        const float ocv = open_circuit_voltage_from_soc(soc_percent);
        const float target_voltage = battery_terminal_target_voltage(ocv, terminal_current);

        float source_command = 0.0f;
        float sink_command = 0.0f;

        const bool soc_floor_hit = (soc_percent <= MIN_SOC_PERCENT && terminal_current < 0.0f);
        const bool soc_ceil_hit = (soc_percent >= MAX_SOC_PERCENT && terminal_current > 0.0f);
        const bool charge_overcurrent = (terminal_current > MAX_CHARGE_CURRENT_A);
        const bool discharge_overcurrent = (terminal_current < -MAX_DISCHARGE_CURRENT_A);

        if (soc_floor_hit || soc_ceil_hit) {
            integrator = 0.0f;
        } else if (charge_overcurrent || discharge_overcurrent) {
            /*
             * Freeze the integrator while protection is active. This prevents
             * stored error from causing a large duty jump when current returns
             * to the allowed range.
             */
        } else {
            const float error = target_voltage - terminal_voltage;
            integrator = clampf(integrator + error * KI * dt_seconds,
                                -INTEGRATOR_LIMIT,
                                INTEGRATOR_LIMIT);

            const float command = clampf(error * KP + integrator, -1.0f, 1.0f);
            if (command >= 0.0f) {
                source_command = command;
            } else {
                sink_command = -command;
            }
        }

        apply_current_protection(terminal_current, &source_command, &sink_command);

        /*
         * Coulomb counting should reflect current that is allowed by the
         * emulator. When protection is active, we conservatively stop SOC
         * integration instead of counting an unsafe transient forever.
         */
        const float effective_current =
            (charge_overcurrent || discharge_overcurrent) ? 0.0f : terminal_current;
        update_soc_by_coulomb_counting(effective_current, dt_seconds);

        set_actuators(source_command, sink_command);
        write_c2000_analog_outputs(target_voltage, terminal_current);

        if ((now_tick - last_log_tick) >= pdMS_TO_TICKS(LOG_PERIOD_MS)) {
            last_log_tick = now_tick;
            log_telemetry((uint32_t)(now_tick * portTICK_PERIOD_MS),
                          soc_percent,
                          ocv,
                          target_voltage,
                          terminal_voltage,
                          terminal_current,
                          source_command,
                          sink_command);
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }
}

void app_main(void) {
    configure_adc();
    configure_dac();
    configure_pwm();
    set_actuators(0.0f, 0.0f);
    configure_enable_pin();

    xTaskCreate(battery_emulator_task,
                "battery_emulator",
                4096,
                NULL,
                5,
                NULL);
}
