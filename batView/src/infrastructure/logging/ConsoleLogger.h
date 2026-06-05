#pragma once

#include "core/services/ILogger.h"

namespace batview::infrastructure::logging {

class ConsoleLogger : public core::services::ILogger {
public:
    void Info(const std::string& message) override;
    void Warning(const std::string& message) override;
    void Error(const std::string& message) override;
};

} // namespace batview::infrastructure::logging
