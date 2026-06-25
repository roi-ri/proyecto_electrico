#include "main_functions.h"
#include "comm_with_pc.h"
#include "battery_controller.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

EventGroupHandle_t control_events = NULL;

typedef struct {
    char name[32];
    float vmax;
    float vmin;
    float amax;
    bool configured;
} battery_profile_t;

static battery_profile_t battery_profile = {0};

static bool parse_int_range(const char *text, int min, int max, int *value)
{
    char *endptr = NULL;
    long parsed = 0;

    if (text == NULL || *text == '\0') {
        return false;
    }

    parsed = strtol(text, &endptr, 10);
    if (*endptr != '\0' || parsed < min || parsed > max) {
        return false;
    }

    *value = (int)parsed;
    return true;
}

static bool parse_positive_float(const char *text, float *value)
{
    char *endptr = NULL;
    float parsed = 0.0f;

    if (text == NULL || *text == '\0') {
        return false;
    }

    parsed = strtof(text, &endptr);
    if (*endptr != '\0' || parsed <= 0.0f) {
        return false;
    }

    *value = parsed;
    return true;
}

static void send_invalid_value(uart_port_t uart_num, const char *message)
{
    enviar_datos_pc(uart_num, "#ERROR,INVALID_VALUE,");
    enviar_datos_pc(uart_num, message);
    enviar_datos_pc(uart_num, "\n");
}

static void send_target_status(uart_port_t uart_num, const char *operation, int target_percent)
{
    char message[64];

    snprintf(message, sizeof(message), "#STATUS,REQUEST,%s_HASTA_%d_PERCENT\n",
             operation, target_percent);
    enviar_datos_pc(uart_num, message);
}

static void send_cycle_status(uart_port_t uart_num, int mode, int cycle_count)
{
    char message[64];

    if (mode == 0) {
        enviar_datos_pc(uart_num, "#STATUS,REQUEST,CICLADO_INFINITO\n");
        return;
    }

    snprintf(message, sizeof(message), "#STATUS,REQUEST,CICLAR_%d_CICLOS\n",
             cycle_count);
    enviar_datos_pc(uart_num, message);
}

static void send_command_received_status(uart_port_t uart_num, const char *command)
{
    char message[48];

    snprintf(message, sizeof(message), "#STATUS,RECEIVED,%s\n", command);
    enviar_datos_pc(uart_num, message);
}

static void send_limited_ack(uart_port_t uart_num, const char *ack, TickType_t *last_ack)
{
    TickType_t now = xTaskGetTickCount();

    if (*last_ack == 0 || (now - *last_ack) >= pdMS_TO_TICKS(1000)) {
        enviar_datos_pc(uart_num, ack);
        *last_ack = now;
    }
}

static void send_stop_response(uart_port_t uart_num)
{
    enviar_datos_pc(uart_num, "#ACK,STOP\n");
}

static void poll_control_commands(uart_port_t uart_num)
{
    static TickType_t last_connection_ack = 0;
    uint8_t buf[128];
    int len = 0;

    do {
        len = recibir_linea_pc(uart_num, buf, sizeof(buf), 1);

        if (len > 0 && strstr((char *)buf, "#STOP") != NULL) {
            EventBits_t bits = xEventGroupGetBits(control_events);

            xEventGroupSetBits(control_events, STOP_BIT);
            if (!(bits & STOP_ACK_BIT)) {
                xEventGroupSetBits(control_events, STOP_ACK_BIT);
                send_stop_response(uart_num);
            }
            xEventGroupClearBits(control_events, WORK_BIT);
        } else if (len > 0 && strstr((char *)buf, "#CONNECTION") != NULL) {
            send_limited_ack(uart_num, "#ACK,CONNECTION\n", &last_connection_ack);
            xEventGroupSetBits(control_events, STOP_BIT | STOP_ACK_BIT);
            xEventGroupClearBits(control_events, WORK_BIT);
        }
    } while (len > 0);
}

static int wait_for_serial_message(uart_port_t uart_num)
{
    (void)uart_num;
    vTaskDelay(pdMS_TO_TICKS(250));

    return 0;
}

static int wait_after_command_ack(uart_port_t uart_num)
{
    (void)uart_num;
    vTaskDelay(pdMS_TO_TICKS(800));

    return 0;
}

void stop_listener_task(void *pvParameters)
{
    (void)pvParameters;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int check_stop_requested(uart_port_t uart_num)
{
    poll_control_commands(uart_num);
    EventBits_t bits = xEventGroupGetBits(control_events);

    if (bits & STOP_BIT) {
        if (!(bits & STOP_ACK_BIT)) {
            xEventGroupSetBits(control_events, STOP_ACK_BIT);
            send_stop_response(uart_num);
        }

        xEventGroupClearBits(control_events, STOP_BIT | WORK_BIT | STOP_ACK_BIT);
        return 1;
    }

    return 0;
}

void battery_profile_function(uart_port_t uart_num, char *datos[], int count)
{
    float vmax = 0.0f;
    float vmin = 0.0f;
    float amax = 0.0f;

    if (count != 5) {
        enviar_datos_pc(uart_num, "#ERROR,INVALID_VALUE,Datos Battery incompletos\n");
        return;
    }

    if (strlen(datos[1]) == 0 || strlen(datos[1]) >= sizeof(battery_profile.name)) {
        send_invalid_value(uart_num, "Nombre de bateria invalido");
        return;
    }

    if (!parse_positive_float(datos[2], &vmax) ||
        !parse_positive_float(datos[3], &vmin) ||
        !parse_positive_float(datos[4], &amax) ||
        vmin >= vmax) {
        send_invalid_value(uart_num, "Perfil de bateria invalido");
        return;
    }

    strncpy(battery_profile.name, datos[1], sizeof(battery_profile.name) - 1);
    battery_profile.name[sizeof(battery_profile.name) - 1] = '\0';
    battery_profile.vmax = vmax;
    battery_profile.vmin = vmin;
    battery_profile.amax = amax;
    battery_profile.configured = true;

    enviar_datos_pc(uart_num, "#ACK,Battery\n");
}

void load_function(uart_port_t uart_num, char *datos[], int count)
{
    int target_percent = 0;

    if (count != 2 || !parse_int_range(datos[1], 0, 100, &target_percent)) {
        send_invalid_value(uart_num, "Porcentaje fuera de rango");
        return;
    }

    if (!battery_profile.configured) {
        enviar_datos_pc(uart_num, "#ERROR,NOT_READY,Perfil de bateria no configurado\n");
        return;
    }

    xEventGroupClearBits(control_events, STOP_BIT | STOP_ACK_BIT);
    enviar_datos_pc(uart_num, "#ACK,LOAD\n");
    if (wait_after_command_ack(uart_num)) {
        return;
    }
    send_command_received_status(uart_num, "LOAD");
    if (wait_for_serial_message(uart_num)) {
        return;
    }
    enviar_datos_pc(uart_num, "#STATUS,CHARGING,PROCESS_ACTIVE\n");
    if (wait_for_serial_message(uart_num)) {
        return;
    }

    xEventGroupSetBits(control_events, WORK_BIT);
    send_target_status(uart_num, "CARGAR", target_percent);
    if (wait_for_serial_message(uart_num)) {
        return;
    }
    int completed = battery_controller(uart_num, 25, target_percent, 0,
                                       battery_profile.vmax, battery_profile.vmin,
                                       battery_profile.amax);
    xEventGroupClearBits(control_events, WORK_BIT | STOP_BIT | STOP_ACK_BIT);

    if (completed) {
        enviar_datos_pc(uart_num, "#STATUS,FINISHED,LOAD_COMPLETE\n");
    }
}

void unload_function(uart_port_t uart_num, char *datos[], int count)
{
    int target_percent = 0;

    if (count != 2 || !parse_int_range(datos[1], 0, 100, &target_percent)) {
        send_invalid_value(uart_num, "Porcentaje fuera de rango");
        return;
    }

    if (!battery_profile.configured) {
        enviar_datos_pc(uart_num, "#ERROR,NOT_READY,Perfil de bateria no configurado\n");
        return;
    }

    xEventGroupClearBits(control_events, STOP_BIT | STOP_ACK_BIT);
    enviar_datos_pc(uart_num, "#ACK,UNLOAD\n");
    if (wait_after_command_ack(uart_num)) {
        return;
    }
    send_command_received_status(uart_num, "UNLOAD");
    if (wait_for_serial_message(uart_num)) {
        return;
    }
    enviar_datos_pc(uart_num, "#STATUS,DISCHARGING,PROCESS_ACTIVE\n");
    if (wait_for_serial_message(uart_num)) {
        return;
    }

    xEventGroupSetBits(control_events, WORK_BIT);
    send_target_status(uart_num, "DESCARGAR", target_percent);
    if (wait_for_serial_message(uart_num)) {
        return;
    }
    int completed = battery_controller(uart_num, 26, target_percent, 0,
                                       battery_profile.vmax, battery_profile.vmin,
                                       battery_profile.amax);
    xEventGroupClearBits(control_events, WORK_BIT | STOP_BIT | STOP_ACK_BIT);

    if (completed) {
        enviar_datos_pc(uart_num, "#STATUS,FINISHED,UNLOAD_COMPLETE\n");
    }
}

void cicle_function(uart_port_t uart_num, char *datos[], int count)
{
    int mode = 0;
    int cycle_count = 0;

    if (count != 3 ||
        !parse_int_range(datos[1], 0, 1, &mode) ||
        !parse_int_range(datos[2], 0, 1000000, &cycle_count) ||
        (mode == 0 && cycle_count != 0) ||
        (mode == 1 && cycle_count <= 0)) {
        send_invalid_value(uart_num, "Parametros CICLE invalidos");
        return;
    }

    if (!battery_profile.configured) {
        enviar_datos_pc(uart_num, "#ERROR,NOT_READY,Perfil de bateria no configurado\n");
        return;
    }

    xEventGroupClearBits(control_events, STOP_BIT | STOP_ACK_BIT);
    enviar_datos_pc(uart_num, "#ACK,CICLE\n");
    if (wait_after_command_ack(uart_num)) {
        return;
    }
    send_command_received_status(uart_num, "CICLE");
    if (wait_for_serial_message(uart_num)) {
        return;
    }
    enviar_datos_pc(uart_num, "#STATUS,CYCLING,PROCESS_ACTIVE\n");
    if (wait_for_serial_message(uart_num)) {
        return;
    }

    xEventGroupSetBits(control_events, WORK_BIT);
    send_cycle_status(uart_num, mode, cycle_count);
    if (wait_for_serial_message(uart_num)) {
        return;
    }
    int completed = battery_controller(uart_num, 0, 100, mode == 0 ? -1 : cycle_count,
                                       battery_profile.vmax, battery_profile.vmin,
                                       battery_profile.amax);
    xEventGroupClearBits(control_events, WORK_BIT | STOP_BIT | STOP_ACK_BIT);

    if (completed) {
        enviar_datos_pc(uart_num, "#STATUS,FINISHED,CYCLE_COMPLETE\n");
    }
}

void stop_function(uart_port_t uart_num)
{
    EventBits_t bits = xEventGroupGetBits(control_events);

    xEventGroupSetBits(control_events, STOP_BIT | STOP_ACK_BIT);
    send_stop_response(uart_num);
    xEventGroupClearBits(control_events, WORK_BIT);

    if (!(bits & WORK_BIT)) {
        xEventGroupClearBits(control_events, STOP_BIT | STOP_ACK_BIT);
    }
}

void process_protocol_command(uart_port_t uart_num, char *datos[], int count)
{
    if (count <= 0 || datos[0] == NULL || datos[0][0] != '#') {
        enviar_datos_pc(uart_num, "#ERROR,INVALID_FRAME,Trama invalida\n");
        return;
    }

    if (strcmp(datos[0], "#Battery") == 0) {
        battery_profile_function(uart_num, datos, count);
    } else if (strcmp(datos[0], "#LOAD") == 0) {
        load_function(uart_num, datos, count);
    } else if (strcmp(datos[0], "#UNLOAD") == 0) {
        unload_function(uart_num, datos, count);
    } else if (strcmp(datos[0], "#CICLE") == 0) {
        cicle_function(uart_num, datos, count);
    } else if (strcmp(datos[0], "#STOP") == 0) {
        if (count == 1) {
            stop_function(uart_num);
        } else {
            send_invalid_value(uart_num, "STOP no acepta parametros");
        }
    } else {
        enviar_datos_pc(uart_num, "#ERROR,INVALID_COMMAND,Comando no reconocido\n");
    }
}
