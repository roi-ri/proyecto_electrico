#pragma once

#include <optional>
#include <string>

#include "core/protocol/ProtocolTypes.h"

namespace batview::core::protocol {

class IProtocolCodec {
public:
    virtual ~IProtocolCodec() = default;

    virtual std::string BuildBatteryProfileCommand(const BatteryProfile& profile) const = 0;
    virtual std::string BuildCycleCommand(bool indefiniteMode, int cycleCount) const = 0;
    virtual std::string BuildStopCommand() const = 0;
    virtual std::string BuildDisconnectCommand() const = 0;
    virtual std::string BuildLoadCommand(int targetPercent) const = 0;
    virtual std::string BuildUnloadCommand(int targetPercent) const = 0;

    virtual std::optional<ProtocolMessage> ParseMessage(const std::string& line) const = 0;
};

} // namespace batview::core::protocol
