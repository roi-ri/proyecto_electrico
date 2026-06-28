/*
    - Función para calcular la tensión eléctrica máxima a la que se debe cargar 
      la batería. Este se calcula en base a una formula.

    Entradas de la función:
        - gpio_value: Es el porcentaje al que se quiere cargar la batería (tiene que ver 
          con el valor que se quiere poner en ese pin, este depende del protocolo, y las 
          tensiones y corrientes máximas que necesita la batería, de igual forma).
        - battery: Corresponde a la batería con que se está trabajando.
        - maximum_voltage: Tensión máxima recibida desde el perfil de la interfaz.
        - minimum_voltage: Tensión mínima recibida desde el perfil de la interfaz.
*/

#ifndef MAX_VOLTAGE_H
#define MAX_VOLTAGE_H

float max_voltage(int gpio_value, int battery, float maximum_voltage, float minimum_voltage);

#endif
