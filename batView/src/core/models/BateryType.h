#pragma once

namespace batview::core::models {

/**
 * @brief Tipos genéricos de batería soportados en la etapa inicial.
 *
 * Esta enumeración representa la selección lógica de batería disponible
 * para el usuario dentro de la aplicación. Puede evolucionar en el futuro
 * hacia perfiles con parámetros nominales asociados.
 */
enum class BatteryType {
    Battery1 = 1,
    Battery2 = 2,
    Battery3 = 3,
    Battery4 = 4
};

} // namespace batview::core::models
