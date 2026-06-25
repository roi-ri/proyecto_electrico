#include <stdint.h>
#include <string.h>

#include "comm_with_pc.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "hal/uart_types.h"
#include "main_functions.h"

void app_main(void)
{
    QueueHandle_t uart_queue;
    uint8_t data_receiv[128];
    char *datos[8] = {NULL};
    int conectado = 0;

    esp_log_level_set("*", ESP_LOG_NONE);
    inicializar_uart(UART_NUM_0, &uart_queue);

    control_events = xEventGroupCreate();
    configASSERT(control_events != NULL);
    xTaskCreate(stop_listener_task, "stop_listener", 2048, (void *)(intptr_t)UART_NUM_0, 5, NULL);

    while (1) {
        int length = recibir_linea_pc(UART_NUM_0, data_receiv, sizeof(data_receiv), 100);

        if (length <= 0) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        int count = dividir_trama((char *)data_receiv, datos, 8);

        if (count > 0 && strcmp(datos[0], "#CONNECTION") == 0) {
            conectado = 1;
            xEventGroupClearBits(control_events, STOP_BIT | WORK_BIT);
            enviar_datos_pc(UART_NUM_0, "#ACK,CONNECTION\n");
        } else if (!conectado) {
            enviar_datos_pc(UART_NUM_0, "#ERROR,NOT_READY,Conexion no establecida\n");
        } else {
            process_protocol_command(UART_NUM_0, datos, count);
        }

        memset(data_receiv, 0, sizeof(data_receiv));
        memset(datos, 0, sizeof(datos));
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
