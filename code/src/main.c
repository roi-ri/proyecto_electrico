#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "comm_with_pc.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/adc_types.h"
#include "hal/uart_types.h"
#include "receive.h"
#include "main_functions.h"
#include "battery_controller.h"

void app_main(void)
{
    QueueHandle_t uart_queue;
    esp_log_level_set("*", ESP_LOG_NONE);
    inicializar_uart(UART_NUM_0, &uart_queue);
    uint8_t data_receiv[128];
    char *datos[128] = {NULL};
    int aprov = 0;
    char message[128];
    adc_oneshot_unit_handle_t handle;
    //inicializar la entrada 
    inicializar_entradas(&handle);
    //configurar la entrada
    configurar_entrada(handle, ADC_CHANNEL_7);
    float medida = 0.0f;
    int conectado = 0;
    control_events = xEventGroupCreate();
    configASSERT(control_events != NULL);

    while (1) {

        if (!conectado) {
            if (connection_ready(UART_NUM_0, data_receiv) == 1) {
                conectado = 1; 
            }
        }
        else {
            recibir_datos_pc(UART_NUM_0, 4, datos, data_receiv, "#ACK,DATA\n");

            if (datos[0] != NULL &&
                datos[1] != NULL &&
                datos[2] != NULL &&
                datos[3] != NULL) {

                if (strcmp(datos[0], "#DATA") == 0) {

                    int bateria_seleccionada = atoi(datos[1]);
                    int tipo_bateria = atoi(datos[2]);
                    int funcionalidad = atoi(datos[3]);
                    datos[0] = NULL;
                    datos[1] = NULL;
                    datos[3] = NULL;
                    datos[4] = NULL;

                    if (bateria_seleccionada) {

                        switch (funcionalidad) {
                            case 1: load_function(UART_NUM_0, datos, data_receiv, 25); aprov = 1;   break;
                            case 2: unload_function(UART_NUM_0, datos, data_receiv, 26); aprov = 1; break;
                            case 3: cicle_function(UART_NUM_0, datos, data_receiv); aprov = 1; break;
                            case 4: stop_function(UART_NUM_0, datos, data_receiv);  aprov = 1; break;
                            default: enviar_datos_pc(UART_NUM_0, "Funcionalidad no válida\n"); break;
                        }
                    }
                }
            }
        }

        if (aprov) {
            enviar_datos_pc(UART_NUM_0, "#INFO: Prueba finalizada\n");
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
