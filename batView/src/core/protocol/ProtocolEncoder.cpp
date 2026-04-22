#include "core/protocol/ProtocolEncoder.h"
#include <sstream>

namespace batview::core::protocol {

std::string ProtocolEncoder::EncodeSelection(int batterySelected, int batteryType, int function) const {
    std::ostringstream oss;
    oss << "#DATA," << batterySelected << "," << batteryType << "," << function << "\n";
    return oss.str();
}

std::string ProtocolEncoder::EncodeCycleCommand(int cycleType, int cycleCount) const {
    std::ostringstream oss;
    oss << "#CICLE," << cycleType << "," << cycleCount << "\n";
    return oss.str();
}

std::string ProtocolEncoder::EncodeStopCommand() const {
    return "#STOP\n";
}

std::string ProtocolEncoder::EncodeLoadCommand(int targetPercent) const {
    std::ostringstream oss;
    oss << "#LOAD," << targetPercent << "\n";
    return oss.str();
}

std::string ProtocolEncoder::EncodeUnloadCommand(int targetPercent) const {
    std::ostringstream oss;
    oss << "#UNLOAD," << targetPercent << "\n";
    return oss.str();
}

} // namespace batview::core::protocol
