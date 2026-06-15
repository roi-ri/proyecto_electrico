/*
    Función que implementa el controlador PI encargado de regular la
    corriente de carga de la batería.

    Durante la etapa de carga, el controlador compara la corriente medida
    con la corriente de referencia y ajusta la señal de control para
    mantener la corriente en el valor requerido por la batería.

    Una vez alcanzada la carga completa, el controlador puede mantener una
    corriente reducida de mantenimiento para evitar la descarga de la batería.

    Funcionamiento:
        - Obtiene la corriente medida a partir de los sensores.
        - Calcula el error respecto a la corriente de referencia.
        - Ejecuta el algoritmo PI.
        - Actualiza la señal de control aplicada al DAC correspondiente.

    Entradas:
        - reference_current: Corriente de referencia a la que se debe mantener.

    Salidas:
        - Ninguna. La acción de control se aplica directamente mediante
          el DAC utilizado para la carga.
*/

#ifndef PI_CONTROLLER_H
#define PI_CONTROLLER_H

void PI_controller(float reference_current);

#endif