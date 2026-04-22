#include "infrastructure/csv/CsvExporter.h"
#include <fstream>

namespace batview::infrastructure::csv {

bool CsvExporter::Export(const std::string& filePath,
                          const std::vector<batview::core::models::Measurement>& data) {
    std::ofstream file(filePath);

    if (!file.is_open()) {
        return false;
    }

    // Escribir encabezado
    file << "Timestamp,Voltage,Current\n";

    // Escribir datos
    for (const auto& measurement : data) {
        file << measurement.timestamp << ","
             << measurement.voltage << ","
             << measurement.current << "\n";
    }

    return true;
}

} // namespace batview::infrastructure::csv
