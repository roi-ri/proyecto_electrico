#pragma once

#include "core/protocol/IProtocolCodec.h"

namespace batview::core::protocol {

class TextProtocolCodec : public IProtocolCodec {
public:
    std::string BuildSelectBatteryCommand(bool batterySelected,
                                          int batteryTypeCode,
                                          int functionCode) const override;
    std::string BuildCycleCommand(bool indefiniteMode, int cycleCount) const override;
    std::string BuildStopCommand() const override;
    std::string BuildLoadCommand(int targetPercent) const override;
    std::string BuildUnloadCommand(int targetPercent) const override;

    std::optional<ProtocolMessage> ParseMessage(const std::string& line) const override;
};

} // namespace batview::core::protocol
