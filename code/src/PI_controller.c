#include "PI_controller.h"
#include "receive.h"

#include "esp_adc/adc_oneshot.h"
#include <stdint.h>

#define CHARGE_CURRENT_CHANNEL ADC_CHANNEL_0
#define DISCHARGE_CURRENT_CHANNEL ADC_CHANNEL_6
#define CONTROL_OUTPUT_MIN 0.0f
#define CONTROL_OUTPUT_MAX 255.0f
#define DAC_ACTIVE_MIN 216.0f
#define DAC_ACTIVE_SPAN 39.0f

static float charge_integral = 0.0f;
static float discharge_integral = 0.0f;

static float *select_integral(adc_channel_t current_channel)
{
    if (current_channel == DISCHARGE_CURRENT_CHANNEL) {
        return &discharge_integral;
    }

    return &charge_integral;
}

static uint8_t scale_control_to_dac(float control_output)
{
    float dac_output = 0.0f;

    if (control_output <= CONTROL_OUTPUT_MIN) {
        return 0;
    }

    if (control_output >= CONTROL_OUTPUT_MAX) {
        return 255;
    }

    dac_output = DAC_ACTIVE_MIN + ((DAC_ACTIVE_SPAN / CONTROL_OUTPUT_MAX) * control_output);

    if (dac_output >= 255.0f) {
        return 255;
    }

    return (uint8_t)dac_output;
}

void PI_controller_reset(void)
{
    charge_integral = 0.0f;
    discharge_integral = 0.0f;
}

void PI_controller_reset_channel(adc_channel_t current_channel)
{
    *select_integral(current_channel) = 0.0f;
}

void PI_controller(dac_oneshot_handle_t dac_handle,
                   adc_channel_t current_channel,
                   float reference_current)
{
    adc_oneshot_unit_handle_t handle;
    float measured_pin_voltage = 0.0f;
    float measured_current = 0.0f;
    float *integral = select_integral(current_channel);
    float error = 0.0f;
    float unsaturated_output = 0.0f;
    float saturated_output = 0.0f;
    float saturation_error = 0.0f;
    const float kp = 2.326e-05f;
    const float ki = 1.0000e-01f;
    const float kb = 1.0f;
    const float sample_time_s = 0.1f;
    const float sensor_gain = 10.0f;
    const float sensor_offset = 2.5f;

    if (dac_handle == NULL) {
        return;
    }

    inicializar_entradas(&handle);
    configurar_entrada(handle, current_channel);
    leer_datos(handle, current_channel, &measured_pin_voltage);
    adc_oneshot_del_unit(handle);

    measured_current = (measured_pin_voltage - sensor_offset) * sensor_gain;
    if (measured_current < 0.0f) {
        measured_current = 0.0f;
    }

    error = reference_current - measured_current;
    *integral += error * sample_time_s;

    unsaturated_output = (kp * error) + (ki * (*integral));
    saturated_output = unsaturated_output;

    if (saturated_output > CONTROL_OUTPUT_MAX) {
        saturated_output = CONTROL_OUTPUT_MAX;
    } else if (saturated_output < CONTROL_OUTPUT_MIN) {
        saturated_output = CONTROL_OUTPUT_MIN;
    }

    saturation_error = saturated_output - unsaturated_output;
    *integral += kb * saturation_error * sample_time_s;

    dac_oneshot_output_voltage(dac_handle, scale_control_to_dac(saturated_output));
}
