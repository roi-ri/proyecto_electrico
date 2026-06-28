/*
Funciones encargadas de la obtención de la declaración y de
los datos de los sensores de corriente y de tensión conectados 
en el ESP32, es una función general para poder utilizarse en 
los 3 sensores, por lo que se debe de indicar el canal al cual 
está conectado cada sensor.
*/


#include "receive.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_rom_sys.h"

#define ADC_PIN_MAX_VOLTAGE 3.3f
#define ADC_EXTERNAL_MAX_VOLTAGE 12.0f
#define ADC_DISCARD_SAMPLES 8
#define ADC_AVERAGE_SAMPLES 16
#define ADC_SETTLE_US 100

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
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(handle, channel, &config));
}

//funcion para leer los datos del sensor, la forma de colocar en el main es: leer_datos(handle, canal, &nombrevariable)
void leer_datos(adc_oneshot_unit_handle_t handle, adc_channel_t channel, float *value){

    // se inicializa la función donde se va a guardar el resultado crudo
    int raw = 0;
    int raw_sum = 0;

    esp_rom_delay_us(ADC_SETTLE_US);

    for(int i = 0; i < ADC_DISCARD_SAMPLES; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(handle, channel, &raw));
    }

    esp_rom_delay_us(ADC_SETTLE_US);

    for(int i = 0; i < ADC_AVERAGE_SAMPLES; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(handle, channel, &raw));
        raw_sum += raw;
    }
    
    *value = ((float)raw_sum / (float)ADC_AVERAGE_SAMPLES / 4095.0f) * ADC_PIN_MAX_VOLTAGE;
}

void leer_datos_12v(adc_oneshot_unit_handle_t handle, adc_channel_t channel, float *value)
{
    float pin_voltage = 0.0f;

    leer_datos(handle, channel, &pin_voltage);
    *value = (pin_voltage / ADC_PIN_MAX_VOLTAGE) * ADC_EXTERNAL_MAX_VOLTAGE;
}
