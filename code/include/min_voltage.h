/*
    - Función para calcular la tensión eléctrica mínima permitida para la descarga de
      la batería.

    Entradas de la función:

        - gpio_value: Es el porcentaje al que se quiere descargar la batería.
        - battery: Corresponde a la batería con la que se está trabajando.
        - maximum_voltage: Tensión máxima recibida desde el perfil de la interfaz.
        - minimum_voltage: Tensión mínima recibida desde el perfil de la interfaz.
*/

#ifndef MIN_VOLTAGE_H
#define MIN_VOLTAGE_H

float min_voltage(int gpio_value, int battery, float maximum_voltage, float minimum_voltage);

#endif
