// Se incluyen los header files de las otras funciones realizadas:
#include "battery_controller.h"
#include "max_voltage.h"
#include "monitoring.h"

// Se incluyen los archivos necesarios del ESP32: 
#include "driver/dac_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

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
void battery_controller(int gpio_num, int gpio_value, int cycles) {
    // Se crea una variable para guardar el valor máximo de tensión al que cargar:
    float maximum_voltage;

    // Se crea la variable mode en base a mode_t:
    mode_t mode = INVALIDO;

    // Se crea una referencia para el DAC:
    dac_oneshot_handle_t dac_handle;

    // Se revisa si se está en caso de carga o descarga (25 carga - 26 descarga):
    dac_oneshot_config_t dac_config = {
        .chan_id = (gpio_num == 25) ? DAC_CHAN_0 : DAC_CHAN_1
    };

    // Se inicializa el DAC: 
    dac_oneshot_new_channel(&dac_config, &dac_handle);

    // Se guarda el valor máximo de tensión al que se debe cargar la batería: 
    maximum_voltage = max_voltage(gpio_value, 1);

    // Se agrega un log para ver que sí se obtuvo este valor correctamente: 
    ESP_LOGI(TAG, "Voltaje máximo: %.2f V", maximum_voltage);

    // Se revisa en qué modo se está:
    if(cycles > 0) {
        mode = CICLADO;
    } else if(gpio_num == 25) {
        mode = CARGA;
    } else if(gpio_num == 26) {
        mode = DESCARGA;
    }

    // Casos para los diferentes modos: 
    switch (mode) {

        case CARGA:

            // Se manda un log de que se está cargando:
            ESP_LOGI(TAG, "Cargando batería.");

            // Se colocan los 3.3V máximos del DAC:
            dac_oneshot_output_voltage(dac_handle, 255);

            // Se monitorea continuamente:
            while(1) {
                
                // Delay para no leer millones de datos por segundo (100ms):
                vTaskDelay(pdMS_TO_TICKS(100));

                // Cuando se alcanza el voltaje máximo:
                if(monitoring(maximum_voltage)) {

                    ESP_LOGI(TAG,"Voltaje máximo alcanzado.");

                    // Protocolo de carga CC-CV:
                    for(int value = 255; value >= (255 * 2.0 / 3.3) ;value--) {

                        dac_oneshot_output_voltage(dac_handle, value);
                        
                        // Otra espera:
                        vTaskDelay(pdMS_TO_TICKS(100));
                    }

                    break;
                }
            }

        break;


        case DESCARGA:
            
            // Se manda un log de que se está descargando:
            ESP_LOGI(TAG, "Descargando batería.");
            
            // Se colocan los 3.3V máximos del DAC:
            dac_oneshot_output_voltage(dac_handle, 255);
            
            // Se monitorea continuamente:
            while(1) {
                
                // Se pone un retardo para no leer millones de datos:
                vTaskDelay(pdMS_TO_TICKS(100));

                // Cuando se llega al voltage mínimo:
                if(minimum_voltage_reached()) {
                    // Se manda log de que se llegó al mínimo voltage:
                    ESP_LOGI(TAG, "Voltaje mínimo alcanzado");
            
                    dac_oneshot_output_voltage(dac_handle, 0);

                    break;
                }
            }

        break;


        case CICLADO:
            
            // Se manda un log de que se está ciclando:
            ESP_LOGI(TAG, "Ciclando batería.");

            // Se realizan los ciclos:
            for(int i = 0; i < cycles; i++) {

                // Se envía un log de por cuál ciclo va: 
                ESP_LOGI(TAG, "Ciclo: %d", i + 1);

                // Etapa de carga:
                // Configuración para carga:
                dac_config.chan_id = DAC_CHAN_0;
                dac_oneshot_del_channel(dac_handle);
                dac_oneshot_new_channel(&dac_config, &dac_handle);
                ESP_LOGI(TAG, "Estado de carga (ciclado).");
                // Se colocan los 3.3 V máximos:
                dac_oneshot_output_voltage(dac_handle, 255);
                // Se monitorea hasta llegar al voltaje máximo:
                while(1) {

                    vTaskDelay(pdMS_TO_TICKS(100));

                    if(monitoring(maximum_voltage)) {

                        ESP_LOGI(TAG, "Máximo voltaje alcanzado.");

                        for(int value = 255; value >= (255 * 2.0 / 3.3); value--) {
                            dac_oneshot_output_voltage(dac_handle, value);
                            vTaskDelay(pdMS_TO_TICKS(100));
                        }

                        break;
                    }
                }

       
                // Etapa de descarga: 
                // Configuración para decarga:
                dac_config.chan_id = DAC_CHAN_1;
                dac_oneshot_del_channel(dac_handle);
                dac_oneshot_new_channel(&dac_config, &dac_handle);
                ESP_LOGI(TAG, "Estado de descarga (ciclado).");
                // Se mantiene la descarga:
                dac_oneshot_output_voltage(dac_handle, 255);
                // Se monitorea continuamente:
                 while(1) {

                    vTaskDelay(pdMS_TO_TICKS(100));

                    if(minimum_voltage_reached()) {

                        ESP_LOGI(TAG, "Voltaje mínimo alcanzado");

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
    
}