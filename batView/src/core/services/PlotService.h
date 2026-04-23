#pragma once

#include <memory>
#include <string>
#include <vector>

#include "models/Measurement.h"

namespace batview::core::services {

enum class PlotAxis {
    TimestampMs,
    Voltage,
    Current,
    State,
    CompletedCycles
};

struct PlotRenderResult {
    bool success = false;
    std::vector<unsigned char> pngBytes;
    std::size_t pointCount = 0;
    std::string error;
};

class PlotService {
public:
    PlotService();
    ~PlotService();

    PlotRenderResult RenderPlot(const std::vector<core::models::Measurement>& data,
                                PlotAxis xAxis,
                                PlotAxis yAxis,
                                int width,
                                int height,
                                const std::string& title) const;

    bool ExportPlotCsv(const std::string& filePath,
                       const std::vector<core::models::Measurement>& data,
                       PlotAxis xAxis,
                       PlotAxis yAxis,
                       std::string& outError) const;

    bool ExportPlotMat(const std::string& filePath,
                       const std::vector<core::models::Measurement>& data,
                       PlotAxis xAxis,
                       PlotAxis yAxis,
                       std::string& outError) const;

    bool ExportPlotXlsx(const std::string& filePath,
                        const std::vector<core::models::Measurement>& data,
                        PlotAxis xAxis,
                        PlotAxis yAxis,
                        std::string& outError) const;

    static std::string AxisLabel(PlotAxis axis);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace batview::core::services
