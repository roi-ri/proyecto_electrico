// Se incluyen los header files de las otras funciones realizadas:
#include "battery_controller.h"
#include "PI_controller.h"
#include "comm_with_pc.h"
#include "main_functions.h"
#include "receive.h"

// Se incluyen los archivos necesarios del ESP32:
#include "driver/dac_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
#include <stdbool.h>
#include <stdio.h>

// Etiqueta que se utiliza en el logging para saber que los mensajes vienen de este módulo:
static const char *TAG = "BATTERY_CONTROLLER";

// ADC1 mapping en la devboard:
// VP/GPIO36 -> ADC_CHANNEL_0: corriente de carga
// VN/GPIO39 -> ADC_CHANNEL_3: tension de bateria
// GPIO34    -> ADC_CHANNEL_6: corriente de descarga (reservado, no usado ahora)
#define CHARGE_CURRENT_CHANNEL ADC_CHANNEL_0
#define VOLTAGE_CHANNEL ADC_CHANNEL_3
#define DISCHARGE_CURRENT_CHANNEL ADC_CHANNEL_6
#define CHARGE_DAC_CHANNEL DAC_CHAN_0
#define DISCHARGE_DAC_CHANNEL DAC_CHAN_1
#define ADC_PIN_MAX_VOLTAGE 3.3f
#define BYPASS_PI_FOR_DAC_TEST 1
#define DAC_TEST_OUTPUT 255

_Static_assert(CHARGE_DAC_CHANNEL != DISCHARGE_DAC_CHANNEL,
               "Carga y descarga deben usar DAC distintos");

// Estructura para los modos:
typedef enum {
    CARGA,
    DESCARGA,
    CICLADO,
    INVALIDO
} mode_t;

// Se implementa la función:
static float read_adc_pin_voltage(adc_channel_t channel)
{
    float value = 0.0f;
    adc_oneshot_unit_handle_t handle;

    inicializar_entradas(&handle);
    configurar_entrada(handle, channel);
    leer_datos(handle, channel, &value);
    adc_oneshot_del_unit(handle);

    return value;
}

static float read_battery_voltage(adc_channel_t channel, float battery_min_voltage,
                                  float battery_max_voltage)
{
    float pin_voltage = read_adc_pin_voltage(channel);
    float normalized_voltage = pin_voltage / ADC_PIN_MAX_VOLTAGE;

    (void)battery_min_voltage;

    if(normalized_voltage < 0.0f) {
        normalized_voltage = 0.0f;
    } else if(normalized_voltage > 1.0f) {
        normalized_voltage = 1.0f;
    }

    return normalized_voltage * battery_max_voltage;
}

static float scale_adc_to_current(adc_channel_t channel, float maximum_current)
{
    float pin_voltage = read_adc_pin_voltage(channel);
    float normalized_current = pin_voltage / ADC_PIN_MAX_VOLTAGE;

    if(normalized_current < 0.0f) {
        normalized_current = 0.0f;
    } else if(normalized_current > 1.0f) {
        normalized_current = 1.0f;
    }

    return normalized_current * maximum_current;
}

static float read_charge_current(float maximum_current)
{
    return scale_adc_to_current(CHARGE_CURRENT_CHANNEL, maximum_current);
}

static float read_discharge_current(float maximum_current)
{
    return scale_adc_to_current(DISCHARGE_CURRENT_CHANNEL, maximum_current);
}

static void activate_charge(dac_oneshot_handle_t charge_dac_handle,
                            dac_oneshot_handle_t discharge_dac_handle,
                            float reference_current)
{
    (void)reference_current;

    dac_oneshot_output_voltage(discharge_dac_handle, 0);
#if BYPASS_PI_FOR_DAC_TEST
    dac_oneshot_output_voltage(charge_dac_handle, DAC_TEST_OUTPUT);
#else
    PI_controller(charge_dac_handle, CHARGE_CURRENT_CHANNEL, reference_current);
#endif
    dac_oneshot_output_voltage(discharge_dac_handle, 0);
}

static void activate_discharge(dac_oneshot_handle_t charge_dac_handle,
                               dac_oneshot_handle_t discharge_dac_handle,
                               float reference_current)
{
    (void)reference_current;

    dac_oneshot_output_voltage(charge_dac_handle, 0);
#if BYPASS_PI_FOR_DAC_TEST
    dac_oneshot_output_voltage(discharge_dac_handle, DAC_TEST_OUTPUT);
#else
    PI_controller(discharge_dac_handle, DISCHARGE_CURRENT_CHANNEL, reference_current);
#endif
    dac_oneshot_output_voltage(charge_dac_handle, 0);
}

static void stop_outputs(dac_oneshot_handle_t charge_dac_handle,
                         dac_oneshot_handle_t discharge_dac_handle)
{
    if(charge_dac_handle != NULL) {
        dac_oneshot_output_voltage(charge_dac_handle, 0);
    }

    if(discharge_dac_handle != NULL) {
        dac_oneshot_output_voltage(discharge_dac_handle, 0);
    }
}

static void stop_charge_output(dac_oneshot_handle_t charge_dac_handle,
                               dac_oneshot_handle_t discharge_dac_handle)
{
    if(charge_dac_handle != NULL) {
        dac_oneshot_output_voltage(charge_dac_handle, 0);
    }

    if(discharge_dac_handle != NULL) {
        dac_oneshot_output_voltage(discharge_dac_handle, 0);
    }

    PI_controller_reset_channel(CHARGE_CURRENT_CHANNEL);
}

static void stop_discharge_output(dac_oneshot_handle_t charge_dac_handle,
                                  dac_oneshot_handle_t discharge_dac_handle)
{
    if(discharge_dac_handle != NULL) {
        dac_oneshot_output_voltage(discharge_dac_handle, 0);
    }

    if(charge_dac_handle != NULL) {
        dac_oneshot_output_voltage(charge_dac_handle, 0);
    }

    PI_controller_reset_channel(DISCHARGE_CURRENT_CHANNEL);
}

static void send_telemetry(uart_port_t uart_num, adc_channel_t current_channel, int state,
                           int completed_cycles, float battery_min_voltage,
                           float battery_max_voltage, float maximum_current)
{
    char message[96];
    float current = current_channel == CHARGE_CURRENT_CHANNEL ?
                    read_charge_current(maximum_current) :
                    read_discharge_current(maximum_current);
    float voltage = read_battery_voltage(VOLTAGE_CHANNEL, battery_min_voltage,
                                         battery_max_voltage);
    int64_t timestamp_ms = esp_timer_get_time() / 1000;

    snprintf(message, sizeof(message), "#DATA,%.2f,%.2f,%lld,%d,%d\n",
             voltage, current, (long long)timestamp_ms, state, completed_cycles);
    enviar_datos_pc(uart_num, message);
}

int battery_controller(uart_port_t uart_num, int gpio_num, int gpio_value, int cycles,
                       float vmax, float vmin, float amax)
{
    int completed = 1;
    // Variable para guardar el voltaje máximo:
    float maximum_voltage;

    // Variable para guardar el voltaje mínimo:
    float minimum_voltage;

    // Corriente de referencia para carga:
    float charge_current = amax;
    float discharge_current = amax;

    // Variable de modo:
    mode_t mode = INVALIDO;

    // Referencias para los DAC:
    dac_oneshot_handle_t charge_dac_handle = NULL;
    dac_oneshot_handle_t discharge_dac_handle = NULL;
    bool charge_dac_initialized = false;
    bool discharge_dac_initialized = false;

    // Configuración inicial de los DAC:
    dac_oneshot_config_t charge_dac_config = {
        .chan_id = CHARGE_DAC_CHANNEL
    };
    dac_oneshot_config_t discharge_dac_config = {
        .chan_id = DISCHARGE_DAC_CHANNEL
    };

    // Revisión del modo:
    if(cycles != 0) {
        mode = CICLADO;
    } else if(gpio_num == 25) {
        mode = CARGA;
    } else if(gpio_num == 26) {
        mode = DESCARGA;
    }

    // Voltajes según el modo:
    if(mode == CICLADO) {
        // Voltaje máximo para ciclado:
        maximum_voltage = vmax;

        // Voltaje mínimo para ciclado:
        minimum_voltage = vmin;
    } else {
        float target_voltage = vmin + ((vmax - vmin) * ((float)gpio_value / 100.0f));

        // Cálculo del voltaje objetivo de carga:
        maximum_voltage = target_voltage;

        // Cálculo del voltaje objetivo de descarga:
        minimum_voltage = target_voltage;
    }

    // Logs de la tensión máxima y mínima a trabajar:
    ESP_LOGI(TAG, "Voltaje máximo: %.2f V", maximum_voltage);
    ESP_LOGI(TAG, "Voltaje mínimo: %.2f V", minimum_voltage);

    if(mode != INVALIDO) {
        dac_oneshot_new_channel(&charge_dac_config, &charge_dac_handle);
        charge_dac_initialized = true;
        dac_oneshot_new_channel(&discharge_dac_config, &discharge_dac_handle);
        discharge_dac_initialized = true;
        stop_outputs(charge_dac_handle, discharge_dac_handle);
    }

    // Casos:
    switch(mode) {
        case CARGA:
            // Log para indicar que se está cargando la batería:
            ESP_LOGI(TAG, "Cargando batería.");
            PI_controller_reset();

            // Bucle principal de carga:
            while(1) {
                if(check_stop_requested(uart_num)) {
                    stop_charge_output(charge_dac_handle, discharge_dac_handle);
                    completed = 0;
                    break;
                }

                activate_charge(charge_dac_handle, discharge_dac_handle, charge_current);

                send_telemetry(uart_num, CHARGE_CURRENT_CHANNEL, 1, 0, vmin, vmax, amax);

                if(check_stop_requested(uart_num)) {
                    stop_charge_output(charge_dac_handle, discharge_dac_handle);
                    completed = 0;
                    break;
                }

                vTaskDelay(pdMS_TO_TICKS(100));

#if !BYPASS_PI_FOR_DAC_TEST
                // Monitoreo de la tensión de la batería:
                if(read_battery_voltage(VOLTAGE_CHANNEL, vmin, vmax) >= maximum_voltage) {

                    ESP_LOGI(TAG, "Voltaje máximo alcanzado.");

                    // Apaga completamente la etapa de carga:
                    stop_charge_output(charge_dac_handle, discharge_dac_handle);

                    break;
                }
#endif
            }
        break;


        case DESCARGA:

            // Log para indicar que se está descargando la batería:
            ESP_LOGI(TAG, "Descargando batería.");
            PI_controller_reset_channel(DISCHARGE_CURRENT_CHANNEL);

            // Bucle principal de descarga:
            while(1) {
                if(check_stop_requested(uart_num)) {
                    stop_discharge_output(charge_dac_handle, discharge_dac_handle);
                    completed = 0;
                    break;
                }

                activate_discharge(charge_dac_handle, discharge_dac_handle, discharge_current);

                send_telemetry(uart_num, DISCHARGE_CURRENT_CHANNEL, 0, 0, vmin, vmax, amax);

                if(check_stop_requested(uart_num)) {
                    stop_discharge_output(charge_dac_handle, discharge_dac_handle);
                    completed = 0;
                    break;
                }

                vTaskDelay(pdMS_TO_TICKS(100));

#if !BYPASS_PI_FOR_DAC_TEST
                // Monitoreo de la tensión de la batería:
                if(read_battery_voltage(VOLTAGE_CHANNEL, vmin, vmax) <= minimum_voltage) {

                    ESP_LOGI(TAG, "Voltaje mínimo alcanzado.");

                    stop_discharge_output(charge_dac_handle, discharge_dac_handle);

                    break;
                }
#endif
            }
        break;


        case CICLADO:

            // Log para indicar que se está en ciclado:
            ESP_LOGI(TAG, "Ciclando batería.");

            // Bucle principal de ciclado. cycles < 0 indica ciclado infinito.
            for(int i = 0; cycles < 0 || i < cycles; i++) {

                ESP_LOGI(TAG, "Ciclo: %d", i + 1);

                // ==============================
                //         ETAPA DE CARGA
                // ==============================
                ESP_LOGI(TAG, "Estado de carga (ciclado).");
                PI_controller_reset();

                while(1) {
                    if(check_stop_requested(uart_num)) {
                        stop_charge_output(charge_dac_handle, discharge_dac_handle);
                        dac_oneshot_del_channel(charge_dac_handle);
                        dac_oneshot_del_channel(discharge_dac_handle);
                        return 0;
                    }

                    activate_charge(charge_dac_handle, discharge_dac_handle, charge_current);

                    send_telemetry(uart_num, CHARGE_CURRENT_CHANNEL, 1, i, vmin, vmax, amax);

                    if(check_stop_requested(uart_num)) {
                        stop_charge_output(charge_dac_handle, discharge_dac_handle);
                        dac_oneshot_del_channel(charge_dac_handle);
                        dac_oneshot_del_channel(discharge_dac_handle);
                        return 0;
                    }

                    vTaskDelay(pdMS_TO_TICKS(100));

                    if(read_battery_voltage(VOLTAGE_CHANNEL, vmin, vmax) >= maximum_voltage) {

                        ESP_LOGI(TAG, "Voltaje máximo alcanzado.");

                        // Apaga completamente la etapa de carga:
                        stop_charge_output(charge_dac_handle, discharge_dac_handle);

                        break;
                    }
                }

                // ==============================
                //      ETAPA DE DESCARGA
                // ==============================
                ESP_LOGI(TAG, "Estado de descarga (ciclado).");
                PI_controller_reset_channel(DISCHARGE_CURRENT_CHANNEL);

                while(1) {
                    if(check_stop_requested(uart_num)) {
                        stop_discharge_output(charge_dac_handle, discharge_dac_handle);
                        dac_oneshot_del_channel(charge_dac_handle);
                        dac_oneshot_del_channel(discharge_dac_handle);
                        return 0;
                    }

                    activate_discharge(charge_dac_handle, discharge_dac_handle, discharge_current);

                    send_telemetry(uart_num, DISCHARGE_CURRENT_CHANNEL, 0, i, vmin, vmax, amax);

                    if(check_stop_requested(uart_num)) {
                        stop_discharge_output(charge_dac_handle, discharge_dac_handle);
                        dac_oneshot_del_channel(charge_dac_handle);
                        dac_oneshot_del_channel(discharge_dac_handle);
                        return 0;
                    }

                    vTaskDelay(pdMS_TO_TICKS(100));

                    if(read_battery_voltage(VOLTAGE_CHANNEL, vmin, vmax) <= minimum_voltage) {

                        ESP_LOGI(TAG, "Voltaje mínimo alcanzado.");

                        stop_discharge_output(charge_dac_handle, discharge_dac_handle);

                        break;
                    }
                }
            }
        break;

        case INVALIDO:

            ESP_LOGE(TAG, "Modo de operación inválido.");
            completed = 0;
        break;
    }

    // Libera el DAC:
    stop_outputs(charge_dac_handle, discharge_dac_handle);

    if(charge_dac_initialized) {
        dac_oneshot_del_channel(charge_dac_handle);
    }

    if(discharge_dac_initialized) {
        dac_oneshot_del_channel(discharge_dac_handle);
    }
    return completed;
}
