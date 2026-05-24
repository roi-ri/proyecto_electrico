#include "monitoring_load.h"
#include "receive.h"
#include "esp_adc/adc_oneshot.h"

bool monitoring_load(float voltage, adc_channel_t channel) {
    // Variable donde se almacena el voltaje leído:
    float measured_voltage = 0.0;

    // Handle del ADC:
    adc_oneshot_unit_handle_t handle;

    // Inicialización:
    inicializar_entradas(&handle);

    // Configuración:
    configurar_entrada(handle, channel);

    // Lectura del voltaje proveniente del sensor:
    leer_datos(handle, channel, &measured_voltage);

    // Liberación del ADC:
    adc_oneshot_del_unit(handle);

    // Comparación para el caso de carga:
    if(measured_voltage >= voltage) {
        return true;
    }

    return false;
}