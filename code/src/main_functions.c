#include "main_functions.h"
#include "comm_with_pc.h"
#include "battery_controller.h"
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "driver/uart.h"

/* 
 * Encargado de realizar tareas paralelas
 */
EventGroupHandle_t control_events = NULL;


/* Task que corre en paralelo escuchando únicamente #STOP */
void stop_listener_task(void *pvParameters)
{
    uart_port_t uart_num = (uart_port_t)(intptr_t)pvParameters;
    uint8_t buf[128];

    while (1) {

        EventBits_t bits = xEventGroupGetBits(control_events);

        /*
         * Solo escucha STOP si hay una operación activa.
         * Esto evita que consuma datos del UART cuando no hay trabajo.
         */
        if (bits & WORK_BIT) {

            int len = uart_read_bytes(
                uart_num,
                buf,
                sizeof(buf) - 1,
                pdMS_TO_TICKS(50)
            );

            if (len > 0) {
                buf[len] = '\0';

                /* Eliminar saltos de línea finales: \r o \n */
                while (len > 0 &&
                      (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
                    buf[--len] = '\0';
                }

                if (strcmp((char *)buf, "#STOP") == 0) {
                    xEventGroupSetBits(control_events, STOP_BIT);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


int check_stop_requested(uart_port_t uart_num)
{
    EventBits_t bits = xEventGroupGetBits(control_events);

    if (bits & STOP_BIT) {

        enviar_datos_pc(uart_num, "#ACK,STOP\n");

        /*
         * Como load/unload/cicle hacen return al detectar STOP,
         * se limpian ambos bits aquí.
         */
        xEventGroupClearBits(control_events, STOP_BIT | WORK_BIT);

        return 1;
    }

    return 0;
}

/* 
 *  load_function se recibe #LOAD,<0-100>
 */
void load_function(uart_port_t uart_num, char *datos[], uint8_t data_receiv[128], int gpio_num) {
 
    char message[128];
 
    recibir_datos_pc(uart_num, 2, datos, data_receiv, "#ACK,LOAD\n");
 
    if (datos[0] != NULL && datos[1] != NULL) {
 
        if (strcmp(datos[0], "#LOAD") == 0) {
 
            int carga = atoi(datos[1]);

            snprintf(message, sizeof(message),
                     "Se desea cargar la batería a un: %d %%\n", carga);
            enviar_datos_pc(uart_num, message);

            /*
             * Se habilita el listener de STOP.
             */
            xEventGroupClearBits(control_events, STOP_BIT);
            xEventGroupSetBits(control_events, WORK_BIT);

            //arreglar esto que vaya dentro del WHILE de las de battery_controller
            /*
                if(check_stop_requested(uart_num)) {
                    //apagar todas las salidas
                    return;
                } */ 

                battery_controller(25, carga, 0);
                vTaskDelay(pdMS_TO_TICKS(100));
            
 
            enviar_datos_pc(uart_num, "Carga completada\n");
 
        } else {
            enviar_datos_pc(uart_num, "#ERROR: Se esperaba #LOAD\n");
        }
 
    } else {
        enviar_datos_pc(uart_num, "#ERROR: Datos LOAD incompletos\n");

    }
 
    xEventGroupClearBits(control_events, WORK_BIT | STOP_BIT);
}
/* 
 *  unload_function, se recibe #UNLOAD,<0-100>
 */
void unload_function(uart_port_t uart_num, char *datos[], uint8_t data_receiv[128], int gpio_num) {
 
    char message[128];
 
    recibir_datos_pc(uart_num, 2, datos, data_receiv, "#ACK,UNLOAD\n");
 
    if (datos[0] != NULL && datos[1] != NULL) {
 
        if (strcmp(datos[0], "#UNLOAD") == 0) {
 
            int descarga = atoi(datos[1]);

            snprintf(message, sizeof(message),
                     "Se desea descargar la bateria a un: %d %%\n", descarga);
            enviar_datos_pc(uart_num, message);

            /*
             * Ya se recibieron los parámetros.
             * Desde aquí puede escucharse #STOP en paralelo.
             */
            xEventGroupClearBits(control_events, STOP_BIT);
            xEventGroupSetBits(control_events, WORK_BIT);
            //colocar esto dentro del WHILE del battery_controller
            /*
                if(check_stop_requested(uart_num)) {
                    //apagar todas las salidas
                    return;
                }
                    */
 
                //lógica de descarga
                battery_controller(26, descarga, 0);
 
                vTaskDelay(pdMS_TO_TICKS(100));
            
 
            enviar_datos_pc(uart_num, "Descarga completada\n");
 
        } else {
            enviar_datos_pc(uart_num, "#ERROR: Se esperaba #UNLOAD\n");
        }
 
    } else {
        enviar_datos_pc(uart_num, "#ERROR: Datos UNLOAD incompletos\n");
    }
 
    xEventGroupClearBits(control_events, WORK_BIT | STOP_BIT);
}
 
/*
 *  cicle_function se recibe #CICLE,0,0  o  #CICLE,1,<n>
 */
void cicle_function(uart_port_t uart_num, char *datos[], uint8_t data_receiv[128]) {
 
    char message[128];
 
    recibir_datos_pc(uart_num, 3, datos, data_receiv, "#ACK,CICLE\n");
 
    if (datos[0] != NULL && datos[1] != NULL && datos[2] != NULL) {
 
        if (strcmp(datos[0], "#CICLE") == 0) {
 
            int modo     = atoi(datos[1]);
            int n_ciclos = atoi(datos[2]);
            int ciclo    = 0;
 
            if (modo == 0) {
                enviar_datos_pc(uart_num, "Ciclado infinito configurado\n");
            } else {
                snprintf(message, sizeof(message),
                         "Ciclado finito configurado: %d ciclos\n", n_ciclos);
                enviar_datos_pc(uart_num, message);
            }

            /*
             * Ya se recibió #CICLE,modo,n_ciclos.
             * Desde aquí empieza el trabajo real.
             */
            xEventGroupClearBits(control_events, STOP_BIT);
            xEventGroupSetBits(control_events, WORK_BIT);
 
            while (modo == 0 || ciclo < n_ciclos) {
 
               if (check_stop_requested(uart_num)) {
                
                    //apagar todo
                    return;
                }

                /*
                * lógica de carga del ciclo
                */

                if (check_stop_requested(uart_num)) {
                    //apagar todo
                    return;
                }

                /*
                * lógica de descarga del ciclo
                */

                ciclo++;

                vTaskDelay(pdMS_TO_TICKS(100));
            }
 
            enviar_datos_pc(uart_num, "Ciclado completado\n");
 
        } else {
            enviar_datos_pc(uart_num, "#ERROR: Se esperaba #CICLE\n");
        }
 
    } else {
        enviar_datos_pc(uart_num, "#ERROR: Datos CICLE incompletos\n");
    }
 
    xEventGroupClearBits(control_events, WORK_BIT | STOP_BIT);
}
 
/* 
 *  stop_function se recibe #STOP
 *  Solo setea STOP_BIT. El ACK lo manda la
 *  función de trabajo al detectar el bit.
 */
void stop_function(uart_port_t uart_num, char *datos[], uint8_t data_receiv[128]) {
 
    recibir_datos_pc(uart_num, 1, datos, data_receiv, NULL);
 
    if (datos[0] != NULL) {
 
        if (strcmp(datos[0], "#STOP") == 0) {
            xEventGroupSetBits(control_events, STOP_BIT);
 
        } else {
            enviar_datos_pc(uart_num, "#ERROR: Se esperaba #STOP\n");
        }
 
    } else {
        enviar_datos_pc(uart_num, "#ERROR: Datos STOP incompletos\n");
    }
}

