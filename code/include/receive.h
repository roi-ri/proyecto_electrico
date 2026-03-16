/*
Creación del header file para facilitar la asignación de pines
de cada sensor, así como la obtención de cada dato que se toma
*/


#ifndef RECEIVE_H
#define RECEIVE_H
#include "driver/adc.h"

// función para inicializar un pin como salida
void inicializar_salida(adc1_channel_t channel);

// función encargada de que el sensor lea los datos
void leer_datos(adc1_channel_t channel, float *value);

#endif
