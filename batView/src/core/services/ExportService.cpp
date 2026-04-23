#include "core/services/ExportService.h"

#include <cstdint>
#include <fstream>
#include <memory>
#include <vector>

#include "infrastructure/python/PythonPlotEngine.h"

namespace batview::core::services {

namespace {

struct MatV4Header {
    std::int32_t type;
    std::int32_t mrows;
    std::int32_t ncols;
    std::int32_t imagf;
    std::int32_t namelen;
};

bool WriteMatV4Vector(std::ofstream& file,
                      const std::string& variableName,
                      const std::vector<double>& values,
                      std::string& outError) {
    const MatV4Header header {
        0,
        static_cast<std::int32_t>(values.size()),
        1,
        0,
        static_cast<std::int32_t>(variableName.size() + 1),
    };

    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    file.write(variableName.c_str(), static_cast<std::streamsize>(variableName.size() + 1));
    file.write(reinterpret_cast<const char*>(values.data()),
               static_cast<std::streamsize>(values.size() * sizeof(double)));

    if (!file.good()) {
        outError = "No se pudo escribir la variable '" + variableName + "' en el archivo MAT.";
        return false;
    }

    return true;
}

std::vector<double> ExtractTimestampSeries(const std::vector<core::models::Measurement>& data) {
    std::vector<double> values;
    values.reserve(data.size());
    for (const auto& measurement : data) {
        values.push_back(static_cast<double>(measurement.timestampMs));
    }
    return values;
}

std::vector<double> ExtractVoltageSeries(const std::vector<core::models::Measurement>& data) {
    std::vector<double> values;
    values.reserve(data.size());
    for (const auto& measurement : data) {
        values.push_back(measurement.voltage);
    }
    return values;
}

std::vector<double> ExtractCurrentSeries(const std::vector<core::models::Measurement>& data) {
    std::vector<double> values;
    values.reserve(data.size());
    for (const auto& measurement : data) {
        values.push_back(measurement.current);
    }
    return values;
}

std::vector<double> ExtractStateSeries(const std::vector<core::models::Measurement>& data) {
    std::vector<double> values;
    values.reserve(data.size());
    for (const auto& measurement : data) {
        values.push_back(static_cast<double>(measurement.state));
    }
    return values;
}

std::vector<double> ExtractCompletedCyclesSeries(const std::vector<core::models::Measurement>& data) {
    std::vector<double> values;
    values.reserve(data.size());
    for (const auto& measurement : data) {
        values.push_back(static_cast<double>(measurement.completedCycles));
    }
    return values;
}

} // namespace

class ExportService::Impl {
public:
    infrastructure::python::PythonPlotEngine pythonEngine;
};

ExportService::ExportService(IDataExporter& csvExporter)
    : impl_(std::make_unique<Impl>()), csvExporter_(csvExporter) {}

ExportService::~ExportService() = default;

bool ExportService::ExportToCsv(const std::string& filePath,
                                const std::vector<core::models::Measurement>& data,
                                std::string& outError) const {
    if (!csvExporter_.Export(filePath, data)) {
        outError = "No se pudo exportar el archivo CSV: " + filePath;
        return false;
    }

    outError.clear();
    return true;
}

bool ExportService::ExportToMat(const std::string& filePath,
                                const std::vector<core::models::Measurement>& data,
                                std::string& outError) const {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        outError = "No se pudo abrir el archivo MAT: " + filePath;
        return false;
    }

    if (!WriteMatV4Vector(file, "timestamp_ms", ExtractTimestampSeries(data), outError) ||
        !WriteMatV4Vector(file, "voltage", ExtractVoltageSeries(data), outError) ||
        !WriteMatV4Vector(file, "current", ExtractCurrentSeries(data), outError) ||
        !WriteMatV4Vector(file, "state", ExtractStateSeries(data), outError) ||
        !WriteMatV4Vector(file, "completed_cycles", ExtractCompletedCyclesSeries(data), outError)) {
        return false;
    }

    outError.clear();
    return true;
}

bool ExportService::ExportToXlsx(const std::string& filePath,
                                 const std::vector<core::models::Measurement>& data,
                                 std::string& outError) const {
    return impl_->pythonEngine.ExportMeasurementsToXlsx(filePath, data, outError);
}

} // namespace batview::core::services
