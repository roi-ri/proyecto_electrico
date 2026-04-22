#pragma once

#include <fstream>
#include <mutex>
#include "core/services/ILogger.h"

namespace batview::infrastructure::logging {

/**
 * @brief Implementación simple de logger a archivo.
 */
class FileLogger : public batview::core::services::ILogger {
public:
    explicit FileLogger(const std::string& filePath);
    ~FileLogger() override;

    void Debug(const std::string& message) override;
    void Info(const std::string& message) override;
    void Warning(const std::string& message) override;
    void Error(const std::string& message) override;

private:
    void Write(const std::string& level, const std::string& message);

    std::ofstream stream_;
    std::mutex mutex_;
};

} // namespace batview::infrastructure::logging
