#ifndef MAIN_FUNCTIONS_H
#define MAIN_FUNCTIONS_H
 
#include <stdint.h>
#include "freertos/FreeRTOS.h" 
#include "freertos/event_groups.h"
#include "hal/uart_types.h"
#include "battery_controller.h"
 
/* ── Bits del EventGroup ── */
#define STOP_BIT  ( 1 << 0 )   /* Señal de parada        */
#define WORK_BIT  ( 1 << 1 )   /* Tarea en progreso       */
#define STOP_ACK_BIT  ( 1 << 2 )   /* Respuesta de parada ya manejada */

/* ── Handle global ── */
extern EventGroupHandle_t control_events;

/* ── Task paralelo para escuchar #STOP ── */
void stop_listener_task(void *pvParameters);

/* ── Función auxiliar para revisar STOP_BIT dentro de load/unload/cicle ── */
int check_stop_requested(uart_port_t uart_num);

/**
 * @brief Maneja el perfil de batería recibido desde batView.
 */
void battery_profile_function(uart_port_t uart_num, char *datos[], int count);

/**
 * @brief Maneja la carga de la batería.
 */
void load_function(uart_port_t uart_num, char *datos[], int count);
 
/**
 * @brief Maneja la descarga de la batería.
 */
void unload_function(uart_port_t uart_num, char *datos[], int count);
 
/**
 * @brief Configura el ciclado de la batería.
 */
void cicle_function(uart_port_t uart_num, char *datos[], int count);
 
/**
 * @brief Detiene la operación activa.
 */
void stop_function(uart_port_t uart_num);

/**
 * @brief Procesa una trama completa recibida desde batView.
 */
void process_protocol_command(uart_port_t uart_num, char *datos[], int count);

#endif
