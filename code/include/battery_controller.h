/*
    - Función para elegir entre una salida u otra, ajustar el valor del pin de salida 
      digital-analógico y ajustarse según el protocolo de carga que utilice la batería.

    Entradas de la función:
        - uart_num: Es el puerto UART utilizado para reportar estado y telemetría.
        - gpio_num: Es el número de pin a utilizar (25 para carga y 26 para descarga).
        - gpio_value: Es el porcentaje al que se quiere cargar la batería (tiene que ver 
          con el tiempo en que se va a poner el valor en ese pin, este depende del protocolo, 
          y las tensiones y corrientes máximas que necesita la batería, de igual forma).
        - cycles: Es la cantidad de ciclos a realizar para estudiar las baterías (en caso
        de ciclado).
        - vmax, vmin y amax: Son los límites recibidos desde el perfil de batView.
*/ 

#ifndef BATTERY_CONTROLLER_H
#define BATTERY_CONTROLLER_H

#include "hal/uart_types.h"

int battery_controller(uart_port_t uart_num, int gpio_num, int gpio_value, int cycles,
                       float vmax, float vmin, float amax);

#endif
