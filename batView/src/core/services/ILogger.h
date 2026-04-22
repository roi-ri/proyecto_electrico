#pragma once

#include <string>

namespace batview::core::services {

/**
 * @brief Interfaz de logging del sistema.
 */
class ILogger {
public:
    virtual ~ILogger() = default;

    virtual void Debug(const std::string& message) = 0;
    virtual void Info(const std::string& message) = 0;
    virtual void Warning(const std::string& message) = 0;
    virtual void Error(const std::string& message) = 0;
};

} // namespace batview::core::services
