/*
    Función encargada de detener completamente la etapa de carga.
    Esta función se utiliza cuando el sistema detecta que la batería
    ha alcanzado el voltaje máximo definido.
    Entradas:
        - Ninguna.
    Funcionamiento:
        - Fuerza la salida del DAC de carga a 0.
        - Libera o reinicia el canal de salida utilizado para la carga.
*/

#ifndef STOP_LOAD_H
#define STOP_LOAD_H

void stop_load(void);

#endif