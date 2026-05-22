/* En este .c se encuentran las funciones utilizadas para la recepción de datos 
 * provenientes de la computadora
 */

#include "comm_with_pc.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/uart_types.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// función para inicializar el puerto uart
void inicializar_uart(uart_port_t uart_num, QueueHandle_t *uart_queue){
    
    // el tamaño del buffer en donde se van a ir guardando los datos que llegan por UART 
    const int uart_buffer_size = (1024 * 2);

    //función para inicializar la uart, donde es el puerto uart, el tamaño de RX, el tamaño de TX
    ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size, uart_buffer_size, 10, uart_queue, 0));
    
    // configurar la uart 
    uart_config_t uart_config = {
        
        .baud_rate = 115200, //velocidad a la que va a enviar datos, 115200 bits por segundo
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,  
    };
    // Se configuran los parámetros de la UART

    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    uart_set_loop_back(uart_num, false); //evitar loopback 
}

//función para enviar datos hacia la computadora 
void enviar_datos_pc(uart_port_t uart_num, const char *message) {
    
    uart_write_bytes(uart_num, message, strlen(message));
}

//función para confirmar la conexión con la PC 

int connection_ready(uart_port_t uart_num, uint8_t data_receiv[128]) {
    
    size_t available = 0;
    uart_get_buffered_data_len(uart_num, &available);

    if (available == 0) {
        return 0;
    }

    int length = uart_read_bytes(uart_num, data_receiv, 127, pdMS_TO_TICKS(100)); //espera 100 milisegundos para recibir datos

    if (length > 0) {
        data_receiv[length] = '\0';

        if (length > 0 && (data_receiv[length - 1] == '\n' || data_receiv[length - 1] == '\r')) {
            data_receiv[length - 1] = '\0';
            length--;
        }
        if (strcmp((char *)data_receiv, "#CONNECTION") == 0) {
            enviar_datos_pc(uart_num, "#ACK,CONNECTION\n");
            memset(data_receiv, 0, 128);
            return 1;
        } else {
            enviar_datos_pc(uart_num, "#ERROR:CODIGO INVALIDO\n");
        }
    }
    memset(data_receiv, 0, 128);
    return 0;
}

//función para recibir datos desde la computadora 
void recibir_datos_pc(uart_port_t uart_num, int SIZE, char *datos[128], uint8_t data_receiv[128], const char *message) { 

    // Limpiar datos anteriores
    for (int i = 0; i < SIZE + 1; i++) datos[i] = NULL;
    // si se recibieron bytes entonces leerlos y guardarlos en data, además de revisar que está completo
    // el envío de datos
    int length = uart_read_bytes(uart_num, data_receiv, 127, pdMS_TO_TICKS(500)); // ← más tiempo

    if (length > 0 && (data_receiv[length - 1] == '\n' || data_receiv[length - 1] == '\r')) {

        //caso de seguridad para evitar overflow
        if (length >= 128) length = 127;
        
        data_receiv[length - 1] = '\0';
        length--;

        // depende del tamaño esperado se van a guardar diferentes cantidades de datos, por esto 
        // es importante colocar el SIZE, según el caso 
        switch (SIZE) {
            case 1:
                datos[0] = strtok((char *)data_receiv, ",");
                // por si se llegan a enviar más datos de lo esperado, que no se guarden los que no se esperaban
                datos[1] = strtok(NULL, ",");
                break;
            case 2:
                datos[0] = strtok((char *)data_receiv, ",");
                datos[1] = strtok(NULL, ",");
                // en caso de recibir más datos de lo esperado guardarlos aquí
                datos[2] = strtok(NULL, ",");
                break;
            case 3:
                datos[0] = strtok((char *)data_receiv, ",");
                datos[1] = strtok(NULL, ",");
                datos[2] = strtok(NULL, ",");
                datos[3] = strtok(NULL, ","); // en caso de recibir más de lo esperado, guardar la basura aquí
                break;
            case 4:
                datos[0] = strtok((char *)data_receiv, ",");
                datos[1] = strtok(NULL, ",");
                datos[2] = strtok(NULL, ",");
                datos[3] = strtok(NULL, ",");
                datos[4] = strtok(NULL, ","); // en caso de recibir más de lo esperado, guardar la basura aquí
                break;
            default:
                enviar_datos_pc(uart_num, "#ERROR: Caso no disponible\n");
                return;
        }

        // ACK solo si hay datos válidos y message no es NULL
        if (message != NULL && datos[0] != NULL) {
            vTaskDelay(pdMS_TO_TICKS(10)); // pequeño delay para que el TX esté listo
            enviar_datos_pc(uart_num, message);
        }

    } else if (length > 0) {
        // Llegaron datos pero sin terminador (\n o \r)
        uart_flush_input(uart_num);
        enviar_datos_pc(uart_num, "#ERROR: Mensaje incompleto\n");
    }
}
