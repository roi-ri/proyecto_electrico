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

static char *find_protocol_command(char *line)
{
    char *command = NULL;

    command = strstr(line, "#Battery");
    if (command != NULL) {
        return command;
    }

    command = strstr(line, "#LOAD");
    if (command != NULL) {
        return command;
    }

    command = strstr(line, "#UNLOAD");
    if (command != NULL) {
        return command;
    }

    command = strstr(line, "#CICLE");
    if (command != NULL) {
        return command;
    }

    return NULL;
}

static int consume_disconect_state(void)
{
    EventBits_t bits = xEventGroupGetBits(control_events);

    if (bits & DISCONECT_BIT) {
        xEventGroupClearBits(control_events, DISCONECT_BIT | STOP_BIT | STOP_ACK_BIT | WORK_BIT);
        return 1;
    }

    return 0;
}

static int process_received_line(uart_port_t uart_num, char *line, char *datos[], int conectado)
{
    char *command_start = find_protocol_command(line);

    if (strstr(line, "#DISCONECT") != NULL || strstr(line, "#DISCONNECT") != NULL) {
        int count = dividir_trama(line, datos, 8);
        process_protocol_command(uart_num, datos, count);
        return 2;
    }

    if (strstr(line, "#STOP") != NULL) {
        stop_function(uart_num);
        return 1;
    }

    if (strstr(line, "#CONNECTION") != NULL) {
        if (!conectado) {
            send_ack_response(uart_num, "#ACK,CONNECTION\n");
        }

        if (command_start == NULL) {
            return 0;
        }

        int count = dividir_trama(command_start, datos, 8);
        process_protocol_command(uart_num, datos, count);
        return 1;
    }

    if (command_start != NULL) {
        int count = dividir_trama(command_start, datos, 8);
        process_protocol_command(uart_num, datos, count);
        return 1;
    }

    if (!conectado) {
        enviar_datos_pc(uart_num, "#ERROR,NOT_READY,Conexion no establecida\n");
        return 0;
    }

    int count = dividir_trama(line, datos, 8);
    process_protocol_command(uart_num, datos, count);
    return 1;
}

static void drain_connection_burst(uart_port_t uart_num, char *datos[])
{
    uint8_t queued_line[128];
    TickType_t start = xTaskGetTickCount();

    while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(250)) {
        int length = recibir_linea_pc(uart_num, queued_line, sizeof(queued_line), 5);

        if (length <= 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
            continue;
        }

        if (process_received_line(uart_num, (char *)queued_line, datos, 1)) {
            break;
        }

        memset(queued_line, 0, sizeof(queued_line));
        memset(datos, 0, sizeof(char *) * 8);
    }
}

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

        char *line = (char *)data_receiv;

        if (strstr(line, "#DISCONECT") != NULL || strstr(line, "#DISCONNECT") != NULL) {
            int count = dividir_trama(line, datos, 8);
            process_protocol_command(UART_NUM_0, datos, count);
            conectado = 0;
        } else if (strstr(line, "#STOP") != NULL) {
            stop_function(UART_NUM_0);
        } else if (strstr(line, "#CONNECTION") != NULL) {
            char *command_start = find_protocol_command(line);

            if (conectado) {
                if (command_start != NULL) {
                    int count = dividir_trama(command_start, datos, 8);
                    process_protocol_command(UART_NUM_0, datos, count);
                }
                memset(data_receiv, 0, sizeof(data_receiv));
                memset(datos, 0, sizeof(datos));
                vTaskDelay(pdMS_TO_TICKS(50));
                continue;
            }

            send_ack_response(UART_NUM_0, "#ACK,CONNECTION\n");

            xEventGroupClearBits(control_events, STOP_BIT | STOP_ACK_BIT | WORK_BIT);
            conectado = 1;

            if (command_start != NULL) {
                int count = dividir_trama(command_start, datos, 8);
                process_protocol_command(UART_NUM_0, datos, count);
            } else {
                drain_connection_burst(UART_NUM_0, datos);
            }
            if (consume_disconect_state()) {
                conectado = 0;
            }
        } else if (!conectado) {
            enviar_datos_pc(UART_NUM_0, "#ERROR,NOT_READY,Conexion no establecida\n");
        } else {
            char *command_start = find_protocol_command(line);
            int count = dividir_trama(command_start != NULL ? command_start : line, datos, 8);
            process_protocol_command(UART_NUM_0, datos, count);
            if (consume_disconect_state()) {
                conectado = 0;
            }
        }

        memset(data_receiv, 0, sizeof(data_receiv));
        memset(datos, 0, sizeof(datos));
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
