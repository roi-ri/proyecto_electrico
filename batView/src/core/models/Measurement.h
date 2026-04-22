#pragma once

#include <cstdint>
#include <optional>

namespace batview::core::models {

/**
 * @brief Representa una medición adquirida durante una sesión.
 *
 * Esta estructura es la unidad base de datos del dominio. Puede provenir
 * del ESP32 o de la importación de un archivo.
 */
struct Measurement {
    double voltage = 0.0;
    double current = 0.0;
    std::int64_t timestamp = 0;
    std::optional<int> state;
    std::optional<int> cycleCount;
};

} // namespace batview::core::models
