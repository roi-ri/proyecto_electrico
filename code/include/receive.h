//Creación del header file para facilitar la asignación de pines
//de cada sensor, así como la obtención de cada dato que se toma

#ifndef RECEIVE_H
#define RECEIVE_H

#include "esp_adc/adc_oneshot.h"

// función para inicializar los pines que reciben ADC1  
void inicializar_entradas(adc_oneshot_unit_handle_t *handle);

// funcion para configurar la entrada 
void configurar_entrada(adc_oneshot_unit_handle_t handle, adc_channel_t channel);

// función encargada de que se lea los datos provenientes del sensor 
void leer_datos(adc_oneshot_unit_handle_t handle, adc_channel_t channel, float *value);
#endif
