#include "max_voltage.h"

float max_voltage(int gpio_value, int battery)
{
    // Variable temporalmente sin uso (se utiliza sólo una batería):
    (void)battery;

    // Tensión máxima, correspondiente al 100% de carga de la batería:
    float maximum_voltage = 13.9;

    // Tensión nominal de la batería:
    float nominal_voltage = 12.0;

    // Diferencia máxima:
    float bias_100;

    // Diferencia según porcentaje:
    float bias_percentage;

    // Voltaje final a monitorear:
    float monitoring_voltage;

    // Cálculo de la diferencia máxima:
    bias_100 = maximum_voltage - nominal_voltage;

    // Ajuste/Diferencia según porcentaje al que se requiere cargar:
    bias_percentage = (bias_100 * gpio_value) / 100.0;

    // Voltaje objetivo:
    monitoring_voltage = nominal_voltage + bias_percentage;

    // Retorno de la tensión a monitorear:
    return monitoring_voltage;
}