#include "core/services/PlotService.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <utility>

#include "infrastructure/python/PythonPlotEngine.h"

namespace batview::core::services {

namespace {

double ExtractAxisValue(const core::models::Measurement& measurement, PlotAxis axis) {
    switch (axis) {
    case PlotAxis::TimestampMs:
        return static_cast<double>(measurement.timestampMs) / 1000.0;
    case PlotAxis::Voltage:
        return measurement.voltage;
    case PlotAxis::Current:
        return measurement.current;
    case PlotAxis::State:
        return static_cast<double>(measurement.state);
    case PlotAxis::CompletedCycles:
        return static_cast<double>(measurement.completedCycles);
    }

    return 0.0;
}

std::pair<std::vector<double>, std::vector<double>> ExtractSeries(
    const std::vector<core::models::Measurement>& data,
    PlotAxis xAxis,
    PlotAxis yAxis) {
    std::vector<double> xValues;
    std::vector<double> yValues;
    xValues.reserve(data.size());
    yValues.reserve(data.size());

    for (const auto& measurement : data) {
        xValues.push_back(ExtractAxisValue(measurement, xAxis));
        yValues.push_back(ExtractAxisValue(measurement, yAxis));
    }

    return {std::move(xValues), std::move(yValues)};
}

bool WriteMatV4Vector(std::ofstream& file,
                      const std::string& variableName,
                      const std::vector<double>& values,
                      std::string& outError) {
    struct MatV4Header {
        std::int32_t type;
        std::int32_t mrows;
        std::int32_t ncols;
        std::int32_t imagf;
        std::int32_t namelen;
    };

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

} // namespace

class PlotService::Impl {
public:
    infrastructure::python::PythonPlotEngine pythonEngine;
};

PlotService::PlotService()
    : impl_(std::make_unique<Impl>()) {}

PlotService::~PlotService() = default;

PlotRenderResult PlotService::RenderPlot(const std::vector<core::models::Measurement>& data,
                                         PlotAxis xAxis,
                                         PlotAxis yAxis,
                                         int width,
                                         int height,
                                         const std::string& title) const {
    const auto [xValues, yValues] = ExtractSeries(data, xAxis, yAxis);
    auto result = impl_->pythonEngine.RenderPlotPng(
        xValues,
        yValues,
        width,
        height,
        AxisLabel(xAxis),
        AxisLabel(yAxis),
        title);
    result.pointCount = xValues.size();
    return result;
}

bool PlotService::ExportPlotCsv(const std::string& filePath,
                                const std::vector<core::models::Measurement>& data,
                                PlotAxis xAxis,
                                PlotAxis yAxis,
                                std::string& outError) const {
    const auto [xValues, yValues] = ExtractSeries(data, xAxis, yAxis);
    std::error_code error;
    const std::filesystem::path path(filePath);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
    }

    std::ofstream file(filePath);
    if (!file.is_open()) {
        outError = "No se pudo abrir el archivo CSV: " + filePath;
        return false;
    }

    file << AxisLabel(xAxis) << "," << AxisLabel(yAxis) << "\n";
    for (std::size_t i = 0; i < xValues.size(); ++i) {
        file << xValues[i] << "," << yValues[i] << "\n";
    }

    if (!file.good()) {
        outError = "No se pudo escribir el archivo CSV: " + filePath;
        return false;
    }

    return true;
}

bool PlotService::ExportPlotMat(const std::string& filePath,
                                const std::vector<core::models::Measurement>& data,
                                PlotAxis xAxis,
                                PlotAxis yAxis,
                                std::string& outError) const {
    const auto [xValues, yValues] = ExtractSeries(data, xAxis, yAxis);
    std::error_code error;
    const std::filesystem::path path(filePath);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
    }

    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        outError = "No se pudo abrir el archivo MAT: " + filePath;
        return false;
    }

    if (!WriteMatV4Vector(file, "x", xValues, outError)) {
        return false;
    }

    if (!WriteMatV4Vector(file, "y", yValues, outError)) {
        return false;
    }

    return true;
}

bool PlotService::ExportPlotXlsx(const std::string& filePath,
                                 const std::vector<core::models::Measurement>& data,
                                 PlotAxis xAxis,
                                 PlotAxis yAxis,
                                 std::string& outError) const {
    const auto [xValues, yValues] = ExtractSeries(data, xAxis, yAxis);
    return impl_->pythonEngine.ExportSeriesToXlsx(
        filePath,
        AxisLabel(xAxis),
        AxisLabel(yAxis),
        xValues,
        yValues,
        outError);
}

std::string PlotService::AxisLabel(PlotAxis axis) {
    switch (axis) {
    case PlotAxis::TimestampMs:
        return "TimeS";
    case PlotAxis::Voltage:
        return "Voltage";
    case PlotAxis::Current:
        return "Current";
    case PlotAxis::State:
        return "State";
    case PlotAxis::CompletedCycles:
        return "CompletedCycles";
    }

    return "Unknown";
}

} // namespace batview::core::services
