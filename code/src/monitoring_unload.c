#include "monitoring_unload.h"
#include "receive.h"
#include "esp_adc/adc_oneshot.h"

bool monitoring_unload(float voltage, adc_channel_t channel) {
    // Variable donde se almacena el voltaje leído:
    float measured_voltage = 0.0;

    // Handle del ADC:
    adc_oneshot_unit_handle_t handle;

    // Inicialización:
    inicializar_entradas(&handle);

    // Configuración:
    configurar_entrada(handle, channel);

    // Lectura del voltaje de batería escalado: 3.3 V en el pin equivale a 12 V.
    leer_datos_12v(handle, channel, &measured_voltage);

    // Liberación del ADC:
    adc_oneshot_del_unit(handle);

    // Comparación para el caso de descarga:
    if(measured_voltage <= voltage) {
        return true;
    }

    return false;
}
