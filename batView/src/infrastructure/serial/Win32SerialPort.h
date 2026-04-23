#pragma once

#include <string>

#include "core/services/ISerialPort.h"

namespace batview::infrastructure::serial {

class Win32SerialPort : public core::services::ISerialPort {
public:
    ~Win32SerialPort() override;

    bool Open(const std::string& portName, int baudRate) override;
    void Close() override;
    bool IsOpen() const override;
    bool WriteLine(const std::string& line) override;
    bool ReadLine(std::string& outLine, int timeoutMs) override;
    const std::string& GetLastError() const override;

private:
    bool ConfigurePort(int baudRate);
    bool WaitForCommEvent(unsigned long timeoutMs) const;
    bool ReadAvailableIntoBuffer(int timeoutMs);
    bool ExtractLineFromBuffer(std::string& outLine);
    void SetLastError(const std::string& message);
    void SetLastErrorFromSystem(const std::string& operation, unsigned long errorCode);
    std::string NormalizePortName(const std::string& portName) const;

    void* handle_ = nullptr;
    std::string portName_;
    std::string readBuffer_;
    std::string lastError_;
};

} // namespace batview::infrastructure::serial
