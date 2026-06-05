#pragma once

#include <string>

namespace batview::core::services {

class ISerialPort {
public:
    virtual ~ISerialPort() = default;

    virtual bool Open(const std::string& portName, int baudRate) = 0;
    virtual void Close() = 0;
    virtual bool IsOpen() const = 0;
    virtual bool WriteLine(const std::string& line) = 0;
    virtual bool ReadLine(std::string& outLine, int timeoutMs) = 0;
    virtual const std::string& GetLastError() const = 0;
};

} // namespace batview::core::services
