#pragma once

#include <memory>
#include <vector>

#include "core/services/IDataExporter.h"

namespace batview::core::services {

class ExportService {
public:
    explicit ExportService(IDataExporter& csvExporter);
    ~ExportService();

    bool ExportToCsv(const std::string& filePath,
                     const std::vector<core::models::Measurement>& data,
                     std::string& outError) const;
    bool ExportToMat(const std::string& filePath,
                     const std::vector<core::models::Measurement>& data,
                     std::string& outError) const;
    bool ExportToXlsx(const std::string& filePath,
                      const std::vector<core::models::Measurement>& data,
                      std::string& outError) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    IDataExporter& csvExporter_;
};

} // namespace batview::core::services
