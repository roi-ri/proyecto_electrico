#pragma once

#include <string>
#include <vector>

#include "models/Measurement.h"
#include "core/services/PlotService.h"

namespace batview::infrastructure::python {

class PythonPlotEngine {
public:
    PythonPlotEngine();
    ~PythonPlotEngine();

    core::services::PlotRenderResult RenderPlotPng(const std::vector<double>& xValues,
                                                   const std::vector<double>& yValues,
                                                   int width,
                                                   int height,
                                                   const std::string& xLabel,
                                                   const std::string& yLabel,
                                                   const std::string& title) const;

    bool ExportSeriesToXlsx(const std::string& filePath,
                            const std::string& xLabel,
                            const std::string& yLabel,
                            const std::vector<double>& xValues,
                            const std::vector<double>& yValues,
                            std::string& outError) const;

    bool ExportMeasurementsToXlsx(const std::string& filePath,
                                  const std::vector<core::models::Measurement>& measurements,
                                  std::string& outError) const;

private:
    bool EnsureInitialized(std::string& outError) const;
    bool EnsureHelpersLoaded(std::string& outError) const;
};

} // namespace batview::infrastructure::python
