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

void send_ack_response(uart_port_t uart_num, const char *ack)
{
    enviar_datos_pc(uart_num, ack);
    vTaskDelay(pdMS_TO_TICKS(30));
    enviar_datos_pc(uart_num, ack);
}

static void send_stop_response(uart_port_t uart_num)
{
    send_ack_response(uart_num, "#ACK,STOP\n");
}

static bool is_disconect_command(const char *text)
{
    return strstr(text, "#DISCONECT") != NULL || strstr(text, "#DISCONNECT") != NULL;
}

static void send_disconect_response(uart_port_t uart_num, const char *text)
{
    (void)text;
    send_ack_response(uart_num, "#ACK,DISCONECT\n");
}

static void poll_control_commands(uart_port_t uart_num)
{
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
        } else if (len > 0 && is_disconect_command((char *)buf)) {
            xEventGroupSetBits(control_events, STOP_BIT | STOP_ACK_BIT | DISCONECT_BIT);
            send_disconect_response(uart_num, (char *)buf);
            xEventGroupClearBits(control_events, WORK_BIT);
            memset(&battery_profile, 0, sizeof(battery_profile));
        }
    } while (len > 0);
}

static int wait_after_operation_ack(uart_port_t uart_num)
{
    const TickType_t total_delay = pdMS_TO_TICKS(150);
    const TickType_t step = pdMS_TO_TICKS(10);
    TickType_t start = xTaskGetTickCount();

    while ((xTaskGetTickCount() - start) < total_delay) {
        if (check_stop_requested(uart_num)) {
            return 1;
        }

        vTaskDelay(step);
    }

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

    send_ack_response(uart_num, "#ACK,Battery\n");
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
    send_ack_response(uart_num, "#ACK,LOAD\n");
    if (wait_after_operation_ack(uart_num)) {
        return;
    }

    xEventGroupSetBits(control_events, WORK_BIT);
    int completed = battery_controller(uart_num, 25, target_percent, 0,
                                       battery_profile.vmax, battery_profile.vmin,
                                       battery_profile.amax);
    xEventGroupClearBits(control_events, WORK_BIT | STOP_BIT | STOP_ACK_BIT);
    (void)completed;
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
    send_ack_response(uart_num, "#ACK,UNLOAD\n");
    if (wait_after_operation_ack(uart_num)) {
        return;
    }

    xEventGroupSetBits(control_events, WORK_BIT);
    int completed = battery_controller(uart_num, 26, target_percent, 0,
                                       battery_profile.vmax, battery_profile.vmin,
                                       battery_profile.amax);
    xEventGroupClearBits(control_events, WORK_BIT | STOP_BIT | STOP_ACK_BIT);
    (void)completed;
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
    send_ack_response(uart_num, "#ACK,CICLE\n");
    if (wait_after_operation_ack(uart_num)) {
        return;
    }

    xEventGroupSetBits(control_events, WORK_BIT);
    int completed = battery_controller(uart_num, 0, 100, mode == 0 ? -1 : cycle_count,
                                       battery_profile.vmax, battery_profile.vmin,
                                       battery_profile.amax);
    xEventGroupClearBits(control_events, WORK_BIT | STOP_BIT | STOP_ACK_BIT);
    (void)completed;
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

void disconect_function(uart_port_t uart_num)
{
    xEventGroupSetBits(control_events, STOP_BIT | STOP_ACK_BIT | DISCONECT_BIT);
    xEventGroupClearBits(control_events, WORK_BIT);
    send_ack_response(uart_num, "#ACK,DISCONECT\n");
    memset(&battery_profile, 0, sizeof(battery_profile));
}

static void disconect_function_with_ack(uart_port_t uart_num, const char *command)
{
    xEventGroupSetBits(control_events, STOP_BIT | STOP_ACK_BIT | DISCONECT_BIT);
    xEventGroupClearBits(control_events, WORK_BIT);
    send_disconect_response(uart_num, command);
    memset(&battery_profile, 0, sizeof(battery_profile));
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
    } else if (strcmp(datos[0], "#DISCONECT") == 0 || strcmp(datos[0], "#DISCONNECT") == 0) {
        if (count == 1) {
            disconect_function_with_ack(uart_num, datos[0]);
        } else {
            send_invalid_value(uart_num, "DISCONNECT no acepta parametros");
        }
    } else {
        enviar_datos_pc(uart_num, "#ERROR,INVALID_COMMAND,Comando no reconocido\n");
    }
}
