idf_component_register(SRCS "esp32_physical_battery_emulator.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_adc_cal)
