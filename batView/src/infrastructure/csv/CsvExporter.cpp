#include "infrastructure/csv/CsvExporter.h"
#include <fstream>

namespace batview::infrastructure::csv {

bool CsvExporter::Export(const std::string& filePath, const std::vector<batview::core::models::Measurement>& data) {
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
