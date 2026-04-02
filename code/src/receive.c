/*
Funciones encargadas de la obtención de la declaración y de
los datos de los sensores de corriente y de tensión conectados 
en el ESP32, es una función general para poder utilizarse en 
los 3 sensores, por lo que se debe de indicar el canal al cual 
está conectado cada sensor.
*/


#include "receive.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"

// función para inicializar los pines que reciben ADC1
void inicializar_entradas(adc_oneshot_unit_handle_t *handle){
    // Inicializar los adc1
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, handle));
}

// función para configurar la entrada 
void configurar_entrada(adc_oneshot_unit_handle_t handle, adc_channel_t channel){
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(handle, channel, &config));
}

//funcion para leer los datos del sensor, la forma de colocar en el main es: leer_datos(handle, canal, &nombrevariable)
void leer_datos(adc_oneshot_unit_handle_t handle, adc_channel_t channel, float *value){

    // se inicializa la función donde se va a guardar el resultado crudo
    int raw = 0;
    //funcion para leer el dato del canal seleccionado
    ESP_ERROR_CHECK(adc_oneshot_read(handle, channel, &raw));
    
    *value = (raw/4095.f) * 3.3f;
}
