/*
Funciones encargadas de la obtención de la declaración y de
los datos de los sensores de corriente y de tensión conectados 
en el ESP32, es una función general para poder utilizarse en 
los 3 sensores, por lo que se debe de indicar el canal al cual 
está conectado cada sensor.
*/


#include "receive.h"
#include "driver/adc.h"

void inicializar_salida(adc1_channel_t channel){
    
    adc1_config_width(ADC_WIDTH_BIT_12); //para poder usar resolución de 12 bits
    adc1_config_channel_atten(channel, ADC_ATTEN_DB_11); //permite medir hasta 3.3V
}


void leer_datos(adc1_channel_t channel, float *value){
    
    if (value == NULL) return;
    
    float a;
    a = adc1_get_raw(channel);
    a = (a/4095.0f) * 3.3f; // para poder tener el valor analógico que envío el sensor
    *value = a; // el valor de a ya se encuentra en el valor que envió el sensor
}
