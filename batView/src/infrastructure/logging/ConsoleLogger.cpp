#include "infrastructure/logging/ConsoleLogger.h"

#include <iostream>

namespace batview::infrastructure::logging {

void ConsoleLogger::Info(const std::string& message) {
    std::cout << "[INFO] " << message << std::endl;
}

void ConsoleLogger::Warning(const std::string& message) {
    std::cout << "[WARN] " << message << std::endl;
}

void ConsoleLogger::Error(const std::string& message) {
    std::cerr << "[ERROR] " << message << std::endl;
}

} // namespace batview::infrastructure::logging
