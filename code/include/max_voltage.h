/*
    - Función para calcular la tensión eléctrica máxima a la que se debe cargar 
      la batería. Este se calcula en base a una formula.

    Entradas de la función:
        - gpio_value: Es el porcentaje al que se quiere cargar la batería (tiene que ver 
          con el valor que se quiere poner en ese pin, este depende del protocolo, y las 
          tensiones y corrientes máximas que necesita la batería, de igual forma).
        - batTery: Corresponde a la batería con que se está trabajando.
*/

#ifndef MAX_VOLTAGE_H
#define MAX_VOLTAGE_H

float max_voltage (int gpio_value, int battery);

#endif