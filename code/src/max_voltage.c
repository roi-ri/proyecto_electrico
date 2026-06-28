#include "max_voltage.h"

float max_voltage(int gpio_value, int battery, float maximum_voltage, float minimum_voltage) {
    // Se revisa que el porcentaje sea válido:
    if(gpio_value < 0) {
        gpio_value = 0;
    }
    if(gpio_value > 100) {
        gpio_value = 100;
    }
    
    // Variable temporalmente sin uso (se utiliza sólo una batería):
    (void)battery;

    // Diferencia máxima:
    float bias_100;

    // Diferencia según porcentaje:
    float bias_percentage;

    // Voltaje final a monitorear:
    float monitoring_voltage;

    // Cálculo de la diferencia máxima:
    bias_100 = maximum_voltage - minimum_voltage;

    // Ajuste/Diferencia según porcentaje al que se requiere cargar:
    bias_percentage = (bias_100 * gpio_value) / 100.0;

    // Voltaje objetivo:
    monitoring_voltage = minimum_voltage + bias_percentage;

    // Retorno de la tensión a monitorear:
    return monitoring_voltage;
}
