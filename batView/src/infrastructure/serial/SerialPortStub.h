#pragma once

#include <queue>
#include <string>

#include "core/services/ISerialPort.h"

namespace batview::infrastructure::serial {

class SerialPortStub : public core::services::ISerialPort {
public:
    bool Open(const std::string& portName, int baudRate) override;
    void Close() override;
    bool IsOpen() const override;
    bool WriteLine(const std::string& line) override;
    bool ReadLine(std::string& outLine, int timeoutMs) override;
    const std::string& GetLastError() const override;

private:
    bool isOpen_ = false;
    std::queue<std::string> frames_;
    std::string lastError_;
};

} // namespace batview::infrastructure::serial
