// Se incluyen los header files de las otras funciones realizadas:
#include "battery_controller.h"
#include "monitoring_load.h"
#include "monitoring_unload.h"
#include "PI_controller.h"
#include "comm_with_pc.h"
#include "main_functions.h"
#include "receive.h"
#include "stop_load.h"

// Se incluyen los archivos necesarios del ESP32:
#include "driver/dac_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
#include <stdio.h>

// Etiqueta que se utiliza en el logging para saber que los mensajes vienen de este módulo:
static const char *TAG = "BATTERY_CONTROLLER";

#define VOLTAGE_CHANNEL ADC_CHANNEL_3
#define CHARGE_CURRENT_CHANNEL ADC_CHANNEL_0
#define DISCHARGE_CURRENT_CHANNEL ADC_CHANNEL_1

// Estructura para los modos:
typedef enum {
    CARGA,
    DESCARGA,
    CICLADO,
    INVALIDO
} mode_t;

// Se implementa la función:
static float read_adc_voltage(adc_channel_t channel)
{
    float value = 0.0f;
    adc_oneshot_unit_handle_t handle;

    inicializar_entradas(&handle);
    configurar_entrada(handle, channel);
    leer_datos(handle, channel, &value);
    adc_oneshot_del_unit(handle);

    return value;
}

static void send_telemetry(uart_port_t uart_num, adc_channel_t current_channel, int state, int completed_cycles)
{
    char message[96];
    float voltage = read_adc_voltage(VOLTAGE_CHANNEL);
    float current = read_adc_voltage(current_channel);
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

    // Variable de modo:
    mode_t mode = INVALIDO;

    // Referencia para el DAC:
    dac_oneshot_handle_t dac_handle;

    // Configuración inicial del DAC:
    dac_oneshot_config_t dac_config = {
        .chan_id = (gpio_num == 25) ? DAC_CHAN_0 : DAC_CHAN_1
    };

    // Inicialización del DAC:
    dac_oneshot_new_channel(&dac_config, &dac_handle);

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

    // Casos:
    switch(mode) {
        case CARGA:
            // Log para indicar que se está cargando la batería:
            ESP_LOGI(TAG, "Cargando batería.");

            // Bucle principal de carga:
            while(1) {
                PI_controller(charge_current);

                send_telemetry(uart_num, CHARGE_CURRENT_CHANNEL, 1, 0);

                if(check_stop_requested(uart_num)) {
                    stop_load();
                    completed = 0;
                    break;
                }

                vTaskDelay(pdMS_TO_TICKS(100));

                // Monitoreo de la tensión de la batería:
                if(monitoring_load(maximum_voltage, VOLTAGE_CHANNEL)) {

                    ESP_LOGI(TAG, "Voltaje máximo alcanzado.");

                    // Apaga completamente la etapa de carga:
                    stop_load();

                    break;
                }
            }
        break;


        case DESCARGA:

            // Log para indicar que se está descargando la batería:
            ESP_LOGI(TAG, "Descargando batería.");

            dac_oneshot_output_voltage(dac_handle, 255);

            // Bucle principal de descarga:
            while(1) {
                send_telemetry(uart_num, DISCHARGE_CURRENT_CHANNEL, 0, 0);

                if(check_stop_requested(uart_num)) {
                    dac_oneshot_output_voltage(dac_handle, 0);
                    completed = 0;
                    break;
                }

                vTaskDelay(pdMS_TO_TICKS(100));

                // Monitoreo de la tensión de la batería:
                if(monitoring_unload(minimum_voltage, VOLTAGE_CHANNEL)) {

                    ESP_LOGI(TAG, "Voltaje mínimo alcanzado.");

                    dac_oneshot_output_voltage(dac_handle, 0);

                    break;
                }
            }
        break;


        case CICLADO:

            // Log para indicar que se está en ciclado:
            ESP_LOGI(TAG, "Ciclando batería.");

            // Bucle principal de ciclado:
            for(int i = 0; cycles < 0 || i < cycles; i++) {

                ESP_LOGI(TAG, "Ciclo: %d", i + 1);

                // ==============================
                //         ETAPA DE CARGA
                // ==============================
                ESP_LOGI(TAG, "Estado de carga (ciclado).");

                while(1) {
                    PI_controller(charge_current);

                    send_telemetry(uart_num, CHARGE_CURRENT_CHANNEL, 1, i);

                    if(check_stop_requested(uart_num)) {
                        stop_load();
                        dac_oneshot_del_channel(dac_handle);
                        return 0;
                    }

                    vTaskDelay(pdMS_TO_TICKS(100));

                    if(monitoring_load(maximum_voltage, VOLTAGE_CHANNEL)) {

                        ESP_LOGI(TAG, "Voltaje máximo alcanzado.");

                        // Apaga completamente la etapa de carga:
                        stop_load();

                        break;
                    }
                }

                // ==============================
                //      ETAPA DE DESCARGA
                // ==============================
                dac_config.chan_id = DAC_CHAN_1;

                dac_oneshot_del_channel(dac_handle);

                dac_oneshot_new_channel(&dac_config, &dac_handle);

                ESP_LOGI(TAG, "Estado de descarga (ciclado).");

                dac_oneshot_output_voltage(dac_handle, 255);

                while(1) {
                    send_telemetry(uart_num, DISCHARGE_CURRENT_CHANNEL, 0, i);

                    if(check_stop_requested(uart_num)) {
                        dac_oneshot_output_voltage(dac_handle, 0);
                        dac_oneshot_del_channel(dac_handle);
                        return 0;
                    }

                    vTaskDelay(pdMS_TO_TICKS(100));

                    if(monitoring_unload(minimum_voltage, VOLTAGE_CHANNEL)) {

                        ESP_LOGI(TAG, "Voltaje mínimo alcanzado.");

                        dac_oneshot_output_voltage(dac_handle, 0);

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
    dac_oneshot_del_channel(dac_handle);
    return completed;
}
