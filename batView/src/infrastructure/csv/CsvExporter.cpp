#include "infrastructure/csv/CsvExporter.h"

#include <filesystem>
#include <fstream>

namespace batview::infrastructure::csv {

bool CsvExporter::Export(const std::string& filePath, const std::vector<batview::core::models::Measurement>& data) {
    std::error_code error;
    const std::filesystem::path path(filePath);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
    }

    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    file << "TimestampMs,Voltage,Current,State,CompletedCycles\n";
    for (const auto& measurement : data) {
        file << measurement.timestampMs << ","
             << measurement.voltage << ","
             << measurement.current << ","
             << measurement.state << ","
             << measurement.completedCycles << "\n";
    }

    return true;
}

} // namespace batview::infrastructure::csv
