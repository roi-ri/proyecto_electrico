#pragma once

#include <string>
#include <vector>

#include "models/Measurement.h"

namespace batview::core::services {

class IDataExporter {
public:
    virtual ~IDataExporter() = default;

    virtual bool Export(const std::string& filePath,
                        const std::vector<core::models::Measurement>& data) = 0;
};

} // namespace batview::core::services
