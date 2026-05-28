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

/* ── Handle global ── */
extern EventGroupHandle_t control_events;

/* ── Task paralelo para escuchar #STOP ── */
void stop_listener_task(void *pvParameters);

/* ── Función auxiliar para revisar STOP_BIT dentro de load/unload/cicle ── */
int check_stop_requested(uart_port_t uart_num);

/**
 * @brief Maneja la carga de la batería.
 */
void load_function(uart_port_t uart_num, char *datos[], uint8_t data_receiv[128], int gpio_num);
 
/**
 * @brief Maneja la descarga de la batería.
 */
void unload_function(uart_port_t uart_num, char *datos[], uint8_t data_receiv[128], int gpio_num);
 
/**
 * @brief Configura el ciclado de la batería.
 */
void cicle_function(uart_port_t uart_num, char *datos[], uint8_t data_receiv[128]);
 
/**
 * @brief Detiene la operación activa.
 */
void stop_function(uart_port_t uart_num, char *datos[], uint8_t data_receiv[128]);

#endif