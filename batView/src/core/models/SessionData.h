#pragma once

#include <mutex>
#include <vector>
#include "Measurement.h"

namespace batview::core::models {

/**
 * @brief Contenedor de datos adquiridos durante una sesión.
 *
 * Esta clase encapsula el almacenamiento en memoria de las mediciones
 * y proporciona una interfaz segura para inserción y consulta.
 */
class SessionData {
public:
    void AddMeasurement(const Measurement& measurement);
    std::vector<Measurement> GetAllMeasurements() const;
    void Clear();
    std::size_t Size() const;

private:
    mutable std::mutex mutex_;
    std::vector<Measurement> measurements_;
};

} // namespace batview::core::models
