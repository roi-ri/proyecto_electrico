#pragma once

#include "core/services/IDataExporter.h"

namespace batview::infrastructure::csv {

class CsvExporter : public batview::core::services::IDataExporter {
public:
    bool Export(const std::string& filePath,
                const std::vector<batview::core::models::Measurement>& data) override;
};

} // namespace batview::infrastructure::csv
