/*
 * Aquí se encuentran las funciones que se van a utilizar para poder 
 * completar la comunicación entre el ESP32 y la computadora en donde
 * se encuentran funcionalidades como inicializar la uart, poder enviar
 * datos/mensajes a la computadora y además se encarga de recibir y filtrar
 * los datos recibidos desde la computadora
 */


#ifndef COMM_WITH_PC_H
#define COMM_WITH_PC_H

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdint.h>
#include <stddef.h>

//función encargada de inicializar el puerto de la uart, así como configurarlo
void inicializar_uart(uart_port_t uart_num, QueueHandle_t *uart_queue);

//función encargada de enviar datos a la computadora
void enviar_datos_pc(uart_port_t uart_num, const char *message);

// función encargada de leer una línea completa desde la computadora
int recibir_linea_pc(uart_port_t uart_num, uint8_t *buffer, size_t buffer_size, uint32_t timeout_ms);

// función encargada de dividir una trama separada por comas
int dividir_trama(char *linea, char *campos[], int max_campos);

// función encargada para confirmar la conexión con la computadora
int connection_ready(uart_port_t uart_num, uint8_t data_receiv[128]);

//función encargada de recibir y filtrar datos desde la computadora 
void recibir_datos_pc(uart_port_t uart_num, int SIZE, char *datos[128], uint8_t data_receiv[128], const char *message);

#endif
