#pragma once

namespace batview::core::models {

/**
 * @brief Modos principales de operación del sistema.
 */
enum class OperationMode {
    Cycle = 1,
    Charge = 2,
    Discharge = 3,
    ImportFile = 4
};

} // namespace batview::core::models
