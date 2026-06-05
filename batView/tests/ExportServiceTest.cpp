#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "core/services/ExportService.h"
#include "infrastructure/csv/CsvExporter.h"

namespace batview::core::services {

namespace {

class TestExporter : public IDataExporter {
public:
    bool Export(const std::string& filePath,
                const std::vector<batview::core::models::Measurement>& data) override {
        lastFilePath = filePath;
        lastCount = data.size();
        return true;
    }

    std::string lastFilePath;
    std::size_t lastCount = 0;
};

} // namespace

TEST(ExportServiceTest, ExportToCsvSuccess) {
    const auto tempFile = std::filesystem::temp_directory_path() / "batview_export_test.csv";
    std::vector<batview::core::models::Measurement> measurements = {
        {123456, 4.12, 0.85, 1, 3},
        {123457, 4.14, 0.90, 0, 4}
    };

    batview::infrastructure::csv::CsvExporter exporter;
    ASSERT_TRUE(exporter.Export(tempFile.string(), measurements));

    std::ifstream file(tempFile);
    ASSERT_TRUE(file.is_open());

    std::string header;
    std::getline(file, header);
    EXPECT_EQ(header, "TimestampMs,Voltage,Current,State,CompletedCycles");

    std::filesystem::remove(tempFile);
}

TEST(ExportServiceTest, ExportServiceDelegatesToCsv) {
    TestExporter exporter;
    ExportService exportService(exporter);

    const std::vector<batview::core::models::Measurement> measurements = {
        {123456, 4.12, 0.85, 1, 3}
    };

    std::string error;
    EXPECT_TRUE(exportService.ExportToCsv("session.csv", measurements, error));
    EXPECT_TRUE(error.empty());
    EXPECT_EQ(exporter.lastFilePath, "session.csv");
    EXPECT_EQ(exporter.lastCount, 1u);
}

TEST(ExportServiceTest, ExportToMatWritesSessionFile) {
    TestExporter exporter;
    ExportService exportService(exporter);
    const auto tempFile = std::filesystem::temp_directory_path() / "batview_session_export.mat";

    const std::vector<batview::core::models::Measurement> measurements = {
        {123456, 4.12, 0.85, 1, 3},
        {123457, 4.14, 0.90, 0, 4}
    };

    std::string error;
    EXPECT_TRUE(exportService.ExportToMat(tempFile.string(), measurements, error));
    EXPECT_TRUE(error.empty());
    EXPECT_TRUE(std::filesystem::exists(tempFile));
    EXPECT_GT(std::filesystem::file_size(tempFile), 0u);

    std::filesystem::remove(tempFile);
}

} // namespace batview::core::services
