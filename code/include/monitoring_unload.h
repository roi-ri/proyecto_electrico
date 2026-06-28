/*
    - Función para monitorear la tensión y conocer cuando se llega al voltaje mínimo permitido
      durante la descarga de la batería. Al llegar a este punto se debe detener la descarga 
      para evitar daños en la batería por subtensión.

    Entradas de la función:
        - voltage: Este corresponde a la salida leída por la función de min_voltage.
        - channel: Corresponde al canal ADC que se está utilizando según el pin asociado al
          proceso de descarga.
    Funcionamiento:
        - Al ser una función booleana, retornará 0 mientras la tensión
          medida sea mayor al límite mínimo establecido.
        - Cuando la tensión medida sea menor o igual al voltaje mínimo
          permitido, retornará 1 para indicar que debe detenerse la descarga.
*/

#ifndef MONITORING_UNLOAD_H
#define MONITORING_UNLOAD_H

#include <stdbool.h>
#include "esp_adc/adc_oneshot.h"

bool monitoring_unload(float voltage, adc_channel_t channel);

#endif