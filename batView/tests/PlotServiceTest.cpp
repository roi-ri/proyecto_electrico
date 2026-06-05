#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "core/services/PlotService.h"

namespace batview::core::services {

namespace {

std::vector<batview::core::models::Measurement> SampleMeasurements() {
    return {
        {1000, 4.10, 0.80, 1, 2},
        {2500, 4.20, 0.95, 0, 3}
    };
}

} // namespace

TEST(PlotServiceTest, ExportPlotCsvUsesSelectedAxes) {
    PlotService plotService;
    const auto tempFile = std::filesystem::temp_directory_path() / "batview_plot_export.csv";

    std::string error;
    ASSERT_TRUE(plotService.ExportPlotCsv(
        tempFile.string(),
        SampleMeasurements(),
        PlotAxis::TimestampMs,
        PlotAxis::Voltage,
        error));
    EXPECT_TRUE(error.empty());

    std::ifstream file(tempFile);
    ASSERT_TRUE(file.is_open());

    std::string header;
    std::getline(file, header);
    EXPECT_EQ(header, "TimeS,Voltage");

    std::string row;
    std::getline(file, row);
    EXPECT_EQ(row, "1,4.1");

    std::filesystem::remove(tempFile);
}

TEST(PlotServiceTest, ExportPlotMatWritesData) {
    PlotService plotService;
    const auto tempFile = std::filesystem::temp_directory_path() / "batview_plot_export.mat";

    std::string error;
    ASSERT_TRUE(plotService.ExportPlotMat(
        tempFile.string(),
        SampleMeasurements(),
        PlotAxis::Current,
        PlotAxis::Voltage,
        error));
    EXPECT_TRUE(error.empty());
    EXPECT_TRUE(std::filesystem::exists(tempFile));
    EXPECT_GT(std::filesystem::file_size(tempFile), 0u);

    std::filesystem::remove(tempFile);
}

TEST(PlotServiceTest, AxisLabelsRemainStable) {
    EXPECT_EQ(PlotService::AxisLabel(PlotAxis::TimestampMs), "TimeS");
    EXPECT_EQ(PlotService::AxisLabel(PlotAxis::CompletedCycles), "CompletedCycles");
}

} // namespace batview::core::services
