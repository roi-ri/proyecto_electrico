#pragma once

#include <optional>
#include <string>
#include "BatteryType.h"
#include "OperationMode.h"

namespace batview::core::models {

/**
 * @brief Configuración activa de una sesión de trabajo.
 *
 * Contiene los parámetros de alto nivel definidos por el usuario y/o por la
 * aplicación antes de iniciar una operación.
 */
struct SessionConfig {
    BatteryType batteryType = BatteryType::Battery1;
    OperationMode operationMode = OperationMode::Cycle;
    int baudRate = 115200;
    std::string serialPortName;
    std::optional<int> targetPercentage;
    std::optional<int> cycleType;
    std::optional<int> cycleCount;
};

} // namespace batview::core::models
