// Se incluyen los header files necesarios:
#include "stop_load.h"
#include "driver/dac_oneshot.h"
#include "esp_log.h"

// Etiqueta para logging:
static const char *TAG = "STOP_LOAD";

// Se implementa la función:
void stop_load(void)
{
    // Handle del DAC:
    dac_oneshot_handle_t dac_handle;

    // Configuración del DAC de carga (GPIO25 -> DAC_CHAN_0):
    dac_oneshot_config_t dac_config = {.chan_id = DAC_CHAN_0};

    // Inicializa el canal del DAC:
    dac_oneshot_new_channel(&dac_config, &dac_handle);

    // Fuerza salida a 0 (apagar carga):
    dac_oneshot_output_voltage(dac_handle, 0);

    // Log de confirmación:
    ESP_LOGI(TAG, "Carga detenida (DAC en 0).");

    // Libera el canal del DAC:
    dac_oneshot_del_channel(dac_handle);
}
