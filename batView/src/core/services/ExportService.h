#pragma once

#include <string>
#include <vector>
#include "core/models/Measurement.h"

namespace batview::core::services {

class ExportService {
public:
    bool ExportToCsv(const std::string& filePath, const std::vector<Measurement>& data);
};

} // namespace batview::core::services
