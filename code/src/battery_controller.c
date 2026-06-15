// Se incluyen los header files de las otras funciones realizadas:
#include "battery_controller.h"
#include "max_voltage.h"
#include "min_voltage.h"
#include "monitoring_load.h"
#include "monitoring_unload.h"
#include "PI_controller.h"
#include "stop_charge.h"

// Se incluyen los archivos necesarios del ESP32:
#include "driver/dac_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

// Etiqueta que se utiliza en el logging para saber que los mensajes vienen de este módulo:
static const char *TAG = "BATTERY_CONTROLLER";

// Estructura para los modos:
typedef enum {
    CARGA,
    DESCARGA,
    CICLADO,
    INVALIDO
} mode_t;

// Se implementa la función:
void battery_controller(int gpio_num, int gpio_value, int cycles)
{
    // Variable para guardar el voltaje máximo:
    float maximum_voltage;

    // Variable para guardar el voltaje mínimo:
    float minimum_voltage;

    // Corriente de referencia para carga:
    float charge_current = 3.0;

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
    if(cycles > 0) {
        mode = CICLADO;
    } else if(gpio_num == 25) {
        mode = CARGA;
    } else if(gpio_num == 26) {
        mode = DESCARGA;
    }

    // Voltajes según el modo:
    if(mode == CICLADO) {
        // Voltaje máximo para ciclado:
        maximum_voltage = 13.9;

        // Voltaje mínimo para ciclado:
        minimum_voltage = 12.0;
    } else {
        // Cálculo del voltaje máximo:
        maximum_voltage = max_voltage(gpio_value, 1);

        // Cálculo del voltaje mínimo:
        minimum_voltage = min_voltage(gpio_value, 1);
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

                vTaskDelay(pdMS_TO_TICKS(100));

                // Monitoreo de la tensión de la batería:
                if(monitoring_load(maximum_voltage, ADC_CHANNEL_3)) {

                    ESP_LOGI(TAG, "Voltaje máximo alcanzado.");

                    // Apaga completamente la etapa de carga:
                    stop_charge();

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
                vTaskDelay(pdMS_TO_TICKS(100));

                // Monitoreo de la tensión de la batería:
                if(monitoring_unload(minimum_voltage, ADC_CHANNEL_3)) {

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
            for(int i = 0; i < cycles; i++) {

                ESP_LOGI(TAG, "Ciclo: %d", i + 1);

                // ==============================
                //         ETAPA DE CARGA
                // ==============================
                ESP_LOGI(TAG, "Estado de carga (ciclado).");

                while(1) {
                    PI_controller(charge_current);

                    vTaskDelay(pdMS_TO_TICKS(100));

                    if(monitoring_load(maximum_voltage, ADC_CHANNEL_3)) {

                        ESP_LOGI(TAG, "Voltaje máximo alcanzado.");

                        // Apaga completamente la etapa de carga:
                        stop_charge();

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
                    vTaskDelay(pdMS_TO_TICKS(100));

                    if(monitoring_unload(minimum_voltage, ADC_CHANNEL_3)) {

                        ESP_LOGI(TAG, "Voltaje mínimo alcanzado.");

                        dac_oneshot_output_voltage(dac_handle, 0);

                        break;
                    }
                }
            }
        break;

        case INVALIDO:

            ESP_LOGE(TAG, "Modo de operación inválido.");
        break;
    }

    // Libera el DAC:
    dac_oneshot_del_channel(dac_handle);
}