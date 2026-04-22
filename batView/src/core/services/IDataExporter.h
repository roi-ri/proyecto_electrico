#pragma once

#include <string>
#include <vector>
#include "core/models/Measurement.h"

namespace batview::core::services {

/**
 * @brief Interfaz genérica para exportación de datos.
 */
class IDataExporter {
public:
    virtual ~IDataExporter() = default;

    virtual bool Export(
        const std::string& filePath,
        const std::vector<batview::core::models::Measurement>& data) = 0;
};

} // namespace batview::core::services
