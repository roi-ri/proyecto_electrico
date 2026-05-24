#include "min_voltage.h"

float min_voltage(int gpio_value, int battery) {
    // Se revisa que el porcentaje sea válido:
    if(gpio_value < 0) {
        gpio_value = 0;
    }
    if(gpio_value > 100) {
        gpio_value = 100;
    }

    // Variable temporalmente sin uso (se utiliza sólo una batería):
    (void)battery;

    // Tensión nominal de la batería descargada:
    float nominal_voltage = 12.0;

    // Tensión máxima de la batería cargada:
    float maximum_voltage = 13.9;

    // Diferencia total:
    float bias_100;

    // Diferencia ajustada:
    float bias_percentage;

    // Voltaje final:
    float monitoring_voltage;

    // Diferencia entre máximo y nominal:
    bias_100 = maximum_voltage - nominal_voltage;

    // Ajuste según porcentaje:
    bias_percentage = (bias_100 * gpio_value) / 100.0;

    // Voltaje objetivo:
    monitoring_voltage = nominal_voltage + bias_percentage;

    // Retorno de la tensión a monitorear:
    return monitoring_voltage;
}