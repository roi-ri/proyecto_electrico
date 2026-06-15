// Se agregan los header files necesarios:
#include "PI_controller.h"
#include "receive.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/dac_oneshot.h"

// Se implementa la función:
void PI_controller(void)
{

    /*=================================================
                    VARIABLES DEL SENSOR
    =================================================*/

    // Corriente medida:
    float measured_current = 0.0;

    // Handle del ADC:
    adc_oneshot_unit_handle_t handle;

    // Canal correspondiente al pin VP (GPIO36):
    adc_channel_t current_channel = ADC_CHANNEL_0;



    /*=================================================
        PARÁMETROS Y VARIABLES DEL CONTROLADOR PI
    =================================================*/

    // Corriente de referencia:
    float reference_current = 3.0;

    // Ganancia proporcional:
    float Kp = 2.3;

    // Ganancia integral:
    float Ki = 1.0;

    // Ganancia del anti-windup:
    float Kb = 1.0;

    // Tiempo de muestreo:
    float Ts = 0.1;

    // Error actual:
    float error = 0.0;

    // Integral acumulada:
    static float integral = 0.0;

    // Salida del controlador sin saturación:
    float Vload_unsat = 0.0;

    // Salida del controlador saturada:
    float Vload_sat = 0.0;

    // Error de saturación para el anti-windup:
    float saturation_error = 0.0;

    // Salida final del controlador:
    float Vload = 0.0;

    // Gancia de la entrada:
    float ganancia_sensor = 10.0;

    // Constante de la entrada:
    float offset_sensor = -2.5;

    // Señal medida modificada:
    float measured_current_modified = 0.0;


    /*=================================================
                    CONFIGURACIÓN DEL ADC
    =================================================*/

    inicializar_entradas(&handle);

    configurar_entrada(handle, current_channel);

    leer_datos(handle, current_channel, &measured_current);

    adc_oneshot_del_unit(handle);



    /*=================================================
                    CONTROLADOR PI
    =================================================*/

    // Calcula la corriente medida modificada:
    measured_current_modified = (measured_current + offset_sensor) * ganancia_sensor;

    // Cálculo del error:
    error = reference_current - measured_current_modified;

    // Salida del PI sin saturación:
    Vload_unsat = (Kp * error) + (Ki * integral);

    // Inicialización de la salida saturada:
    Vload_sat = Vload_unsat;

    // Revisa la saturación superior del DAC:
    if(Vload_sat > 255.0) {
        Vload_sat = 255.0;
    }

    // Revisa la saturación inferior del DAC:
    if(Vload_sat < 0.0) {
        Vload_sat = 0.0;
    }

    // Diferencia entre la salida saturada y la no saturada:
    saturation_error = Vload_sat - Vload_unsat;

    // Acumulación de la integral con anti-windup mediante el método Back Calculation:
    integral = integral + (error * Ts) + (Kb * saturation_error * Ts);

    // Salida final del controlador:
    Vload = Vload_sat;



    /*=================================================
                    ETAPA DE SALIDA
    =================================================*/

    // Configuración del DAC (GPIO25, pin de carga):
    dac_oneshot_handle_t dac_handle;
    dac_oneshot_config_t dac_config = {.chan_id = DAC_CHAN_0};
    dac_oneshot_new_channel(&dac_config, &dac_handle);

    // Actualización de la salida:
    dac_oneshot_output_voltage(dac_handle, (uint8_t)Vload);

    // Libera el DAC:
    dac_oneshot_del_channel(dac_handle);
}