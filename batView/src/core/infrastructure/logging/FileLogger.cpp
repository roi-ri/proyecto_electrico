#include "infrastructure/logging/FileLogger.h"
#include <chrono>
#include <iomanip>

namespace batview::infrastructure::logging {

FileLogger::FileLogger(const std::string& filePath)
    : stream_(filePath, std::ios::app) {}

FileLogger::~FileLogger() {
    if (stream_.is_open()) {
        stream_.close();
    }
}

void FileLogger::Debug(const std::string& message) { Write("DEBUG", message); }
void FileLogger::Info(const std::string& message) { Write("INFO", message); }
void FileLogger::Warning(const std::string& message) { Write("WARN", message); }
void FileLogger::Error(const std::string& message) { Write("ERROR", message); }

void FileLogger::Write(const std::string& level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!stream_.is_open()) {
        return;
    }

    stream_ << "[" << level << "] " << message << '\n';
    stream_.flush();
}

} // namespace batview::infrastructure::logging
