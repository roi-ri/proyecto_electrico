// Se incluyen los header files de las otras funciones realizadas:
#include "battery_controller.h"
#include "max_voltage.h"
#include "min_voltage.h"
#include "monitoring_load.h"
#include "monitoring_unload.h"

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

    // Logs:
    ESP_LOGI(TAG, "Voltaje máximo: %.2f V", maximum_voltage);
    ESP_LOGI(TAG, "Voltaje mínimo: %.2f V", minimum_voltage);

    // Casos:
    switch(mode) {

        case CARGA:

            // 
            ESP_LOGI(TAG, "Cargando batería.");

            // 
            dac_oneshot_output_voltage(dac_handle, 255);

            // 
            while(1) {

                // 
                vTaskDelay(pdMS_TO_TICKS(100));

                // 
                if(monitoring_load(maximum_voltage, ADC_CHANNEL_0)) {

                    // 
                    ESP_LOGI(TAG, "Voltaje máximo alcanzado.");

                    // 
                    for(int value = 255;
                        value >= (255 * 2.0 / 3.3);
                        value--) {

                        // 
                        dac_oneshot_output_voltage(dac_handle, value);

                        // 
                        vTaskDelay(pdMS_TO_TICKS(100));
                    }

                    // 
                    break;
                }
            }

        break;

        case DESCARGA:

            // 
            ESP_LOGI(TAG, "Descargando batería.");

            // 
            dac_oneshot_output_voltage(dac_handle, 255);

            // 
            while(1) {

                // 
                vTaskDelay(pdMS_TO_TICKS(100));

                // 
                if(monitoring_unload(minimum_voltage,
                                     ADC_CHANNEL_1)) {

                    // 
                    ESP_LOGI(TAG, "Voltaje mínimo alcanzado.");

                    // 
                    dac_oneshot_output_voltage(dac_handle, 0);

                    // 
                    break;
                }
            }

        break;

        case CICLADO:

            // 
            ESP_LOGI(TAG, "Ciclando batería.");

            // 
            for(int i = 0; i < cycles; i++) {

                // 
                ESP_LOGI(TAG, "Ciclo: %d", i + 1);

                // ------------------------------
                // ETAPA DE CARGA
                // ------------------------------

                // 
                dac_config.chan_id = DAC_CHAN_0;

                // 
                dac_oneshot_del_channel(dac_handle);

                // 
                dac_oneshot_new_channel(&dac_config,
                                        &dac_handle);

                // 
                ESP_LOGI(TAG,
                         "Estado de carga (ciclado).");

                // 
                dac_oneshot_output_voltage(dac_handle, 255);

                // 
                while(1) {

                    // 
                    vTaskDelay(pdMS_TO_TICKS(100));

                    // 
                    if(monitoring_load(maximum_voltage,
                                       ADC_CHANNEL_0)) {

                        // 
                        ESP_LOGI(TAG,
                                 "Voltaje máximo alcanzado.");

                        // 
                        for(int value = 255;
                            value >= (255 * 2.0 / 3.3);
                            value--) {

                            // 
                            dac_oneshot_output_voltage(dac_handle,
                                                       value);

                            // 
                            vTaskDelay(pdMS_TO_TICKS(100));
                        }

                        // 
                        break;
                    }
                }

                // ------------------------------
                // ETAPA DE DESCARGA
                // ------------------------------

                // 
                dac_config.chan_id = DAC_CHAN_1;

                // 
                dac_oneshot_del_channel(dac_handle);

                // 
                dac_oneshot_new_channel(&dac_config,
                                        &dac_handle);

                // 
                ESP_LOGI(TAG,
                         "Estado de descarga (ciclado).");

                // 
                dac_oneshot_output_voltage(dac_handle, 255);

                // 
                while(1) {

                    // 
                    vTaskDelay(pdMS_TO_TICKS(100));

                    // 
                    if(monitoring_unload(minimum_voltage,
                                         ADC_CHANNEL_1)) {

                        // 
                        ESP_LOGI(TAG,
                                 "Voltaje mínimo alcanzado.");

                        // 
                        dac_oneshot_output_voltage(dac_handle, 0);

                        // 
                        break;
                    }
                }
            }

        break;

        case INVALIDO:

            // 
            ESP_LOGE(TAG,
                     "Modo de operación inválido.");

        break;

    }

    // 
    dac_oneshot_del_channel(dac_handle);
}