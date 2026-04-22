#include "core/services/ExportService.h"
#include "infrastructure/csv/CsvExporter.h"

namespace batview::core::services {

bool ExportService::ExportToCsv(const std::string& filePath, const std::vector<Measurement>& data) {
    CsvExporter exporter;
    return exporter.Export(filePath, data);
}

} // namespace batview::core::services
