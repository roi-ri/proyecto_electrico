#pragma once

#include <functional>
#include <memory>
#include <string>

#include "core/services/ILogger.h"
#include "core/services/ISerialPort.h"

namespace batview::core::services {

class ConnectionService {
public:
    using TrafficObserver = std::function<void(bool, const std::string&)>;

    ConnectionService(ISerialPort& serialPort, ILogger& logger);

    bool Connect(const std::string& portName, int baudRate);
    void Disconnect();
    void SetTrafficObserver(TrafficObserver observer);
    const std::string& GetLastError() const;

private:
    ISerialPort& serialPort_;
    ILogger& logger_;
    TrafficObserver trafficObserver_;
    std::string lastError_;
};

} // namespace batview::core::services
