#pragma once

#include <chrono>
#include <functional>
#include <string>

#include "core/protocol/IProtocolCodec.h"
#include "core/services/ILogger.h"
#include "core/services/ISerialPort.h"

namespace batview::core::services {

class ProtocolSessionService {
public:
    using TrafficObserver = std::function<void(bool, const std::string&)>;

    ProtocolSessionService(ISerialPort& serialPort,
                           core::protocol::IProtocolCodec& protocolCodec,
                           ILogger& logger);

    bool SendBatterySelection(int batteryTypeCode, int functionCode, bool batterySelected,
                              std::chrono::milliseconds timeout);
    bool SendCycleConfiguration(bool indefiniteMode, int cycleCount,
                                std::chrono::milliseconds timeout);
    bool SendStop(std::chrono::milliseconds timeout);
    bool SendLoadTarget(int targetPercent, std::chrono::milliseconds timeout);
    bool SendUnloadTarget(int targetPercent, std::chrono::milliseconds timeout);
    void SetTrafficObserver(TrafficObserver observer);
    const std::string& GetLastError() const;

private:
    bool SendCommandAndAwaitAck(const std::string& command,
                                const std::string& expectedAckToken,
                                std::chrono::milliseconds timeout);

    ISerialPort& serialPort_;
    core::protocol::IProtocolCodec& protocolCodec_;
    ILogger& logger_;
    TrafficObserver trafficObserver_;
    std::string lastError_;
};

} // namespace batview::core::services
