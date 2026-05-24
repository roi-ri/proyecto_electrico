/*
    - Función para monitorear la tensión y conocer cuando se llega a la tensión
      máxima dependiendo del porcentaje elegido. Al llegar a este punto se debe de empezar
      a bajar la tensión en el pin para ir disminuyendo la corriente que llega a la 
      batería. 

    Entradas de la función:
        - voltage: Este corresponde a la salida leída por la función de max_voltage o min_voltage.
    Funcionamiento: 
        - Al ser una función booleana, será cero en todo momento hasta que se llegue al valor
        máximo de tensión eléctrica, donde cambia a 1 para indicar que se debe de empezar 
        a bajar la tensión en el pin correspondiente hasta llegar a un mínimo.
*/

#ifndef MONITORING_H
#define MONITORING_H

#include <stdbool.h>

bool monitoring (float voltage);

#endif