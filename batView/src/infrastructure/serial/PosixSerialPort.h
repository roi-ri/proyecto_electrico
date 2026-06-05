#pragma once

#include <string>

#include "core/services/ISerialPort.h"

namespace batview::infrastructure::serial {

class PosixSerialPort : public core::services::ISerialPort {
public:
    ~PosixSerialPort() override;

    bool Open(const std::string& portName, int baudRate) override;
    void Close() override;
    bool IsOpen() const override;
    bool WriteLine(const std::string& line) override;
    bool ReadLine(std::string& outLine, int timeoutMs) override;
    const std::string& GetLastError() const override;

private:
    bool ConfigurePort(int baudRate);
    bool ReadAvailableIntoBuffer(int timeoutMs);
    bool ExtractLineFromBuffer(std::string& outLine);
    void SetLastError(const std::string& message);
    void SetLastErrorFromErrno(const std::string& operation, const std::string& portName = std::string());

    int fd_ = -1;
    std::string portName_;
    std::string readBuffer_;
    std::string lastError_;
};

} // namespace batview::infrastructure::serial
