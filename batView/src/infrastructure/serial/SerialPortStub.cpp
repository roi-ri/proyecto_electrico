#include "infrastructure/serial/SerialPortStub.h"

#include <string>

namespace batview::infrastructure::serial {

bool SerialPortStub::Open(const std::string& portName, int baudRate) {
    (void)portName;
    (void)baudRate;

    isOpen_ = true;
    lastError_.clear();
    return true;
}

void SerialPortStub::Close() {
    isOpen_ = false;
}

bool SerialPortStub::IsOpen() const {
    return isOpen_;
}

bool SerialPortStub::WriteLine(const std::string& line) {
    if (!isOpen_) {
        lastError_ = "stub write failed: serial port is not open.";
        return false;
    }

    if (line == "#CONNECTION") {
        frames_.push("#ACK,CONECTION");
        frames_.push("#STATUS,CONNECTED,ESP32_READY");
        return true;
    }

    if (line.rfind("#DATA,", 0) == 0) {
        frames_.push("#ACK,DATA");
        frames_.push("#STATUS,CONNECTED,ESP32_READY");
        return true;
    }

    if (line.rfind("#CICLE,", 0) == 0) {
        frames_.push("#ACK,CICLE");
        frames_.push("#STATUS,CYCLING,PROCESS_ACTIVE");
        frames_.push("#DATA,4.12,0.85,1520,1,3");
        frames_.push("#DATA,4.10,0.82,1770,1,3");
        frames_.push("#DATA,4.05,0.90,2020,0,4");
        return true;
    }

    if (line == "#STOP") {
        frames_.push("#ACK,STOP");
        frames_.push("#STATUS,FINISHED,CYCLE_COMPLETE");
        return true;
    }

    if (line.rfind("#LOAD,", 0) == 0) {
        frames_.push("#ACK,LOAD");
        frames_.push("#STATUS,CHARGING,PROCESS_ACTIVE");
        frames_.push("#DATA,3.95,1.10,2100");
        return true;
    }

    if (line.rfind("#UNLOAD,", 0) == 0) {
        frames_.push("#ACK,UNLOAD");
        frames_.push("#STATUS,DISCHARGING,PROCESS_ACTIVE");
        frames_.push("#DATA,3.80,1.05,2200");
        return true;
    }

    frames_.push("#ERROR,01,INVALID_COMMAND");
    return true;
}

bool SerialPortStub::ReadLine(std::string& outLine, int timeoutMs) {
    (void)timeoutMs;

    if (!isOpen_ || frames_.empty()) {
        return false;
    }

    outLine = frames_.front();
    frames_.pop();
    return true;
}

const std::string& SerialPortStub::GetLastError() const {
    return lastError_;
}

} // namespace batview::infrastructure::serial
