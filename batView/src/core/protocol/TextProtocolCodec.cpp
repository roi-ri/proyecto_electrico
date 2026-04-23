#include "core/protocol/TextProtocolCodec.h"

#include <sstream>
#include <stdexcept>
#include <vector>

namespace batview::core::protocol {

namespace {
std::vector<std::string> Split(const std::string& input, char separator) {
    std::vector<std::string> parts;
    std::stringstream ss(input);
    std::string token;

    while (std::getline(ss, token, separator)) {
        parts.push_back(token);
    }

    return parts;
}

MessageType ToMessageType(const std::string& token) {
    if (token == "DATA") {
        return MessageType::Data;
    }
    if (token == "CICLE") {
        return MessageType::Cicle;
    }
    if (token == "STOP") {
        return MessageType::Stop;
    }
    if (token == "STATUS") {
        return MessageType::Status;
    }
    if (token == "ACK") {
        return MessageType::Ack;
    }
    if (token == "ERROR") {
        return MessageType::Error;
    }
    if (token == "LOAD") {
        return MessageType::Load;
    }
    if (token == "UNLOAD") {
        return MessageType::Unload;
    }

    return MessageType::Unknown;
}

bool IsPositivePercent(int value) {
    return value >= 0 && value <= 100;
}
} // namespace

std::string TextProtocolCodec::BuildSelectBatteryCommand(bool batterySelected,
                                                         int batteryTypeCode,
                                                         int functionCode) const {
    if (batteryTypeCode < 1 || batteryTypeCode > 4) {
        throw std::invalid_argument("batteryTypeCode must be in [1,4]");
    }
    if (functionCode < 1 || functionCode > 4) {
        throw std::invalid_argument("functionCode must be in [1,4]");
    }

    return "#DATA," + std::to_string(batterySelected ? 1 : 0) + "," +
           std::to_string(batteryTypeCode) + "," + std::to_string(functionCode);
}

std::string TextProtocolCodec::BuildCycleCommand(bool indefiniteMode, int cycleCount) const {
    if (indefiniteMode) {
        return "#CICLE,0,0";
    }

    if (cycleCount <= 0) {
        throw std::invalid_argument("cycleCount must be > 0 for defined cycle mode");
    }

    return "#CICLE,1," + std::to_string(cycleCount);
}

std::string TextProtocolCodec::BuildStopCommand() const {
    return "#STOP";
}

std::string TextProtocolCodec::BuildLoadCommand(int targetPercent) const {
    if (!IsPositivePercent(targetPercent)) {
        throw std::invalid_argument("targetPercent must be in [0,100]");
    }

    return "#LOAD," + std::to_string(targetPercent);
}

std::string TextProtocolCodec::BuildUnloadCommand(int targetPercent) const {
    if (!IsPositivePercent(targetPercent)) {
        throw std::invalid_argument("targetPercent must be in [0,100]");
    }

    return "#UNLOAD," + std::to_string(targetPercent);
}

std::optional<ProtocolMessage> TextProtocolCodec::ParseMessage(const std::string& line) const {
    if (line.empty() || line[0] != '#') {
        return std::nullopt;
    }

    const auto parts = Split(line.substr(1), ',');
    if (parts.empty()) {
        return std::nullopt;
    }

    ProtocolMessage message;
    message.typeToken = parts[0];
    message.type = ToMessageType(message.typeToken);

    if (message.type == MessageType::Unknown) {
        return std::nullopt;
    }

    for (std::size_t i = 1; i < parts.size(); ++i) {
        message.fields.push_back(parts[i]);
    }

    // Strict field validation by message type.
    if (message.type == MessageType::Stop) {
        return message.fields.empty() ? std::optional<ProtocolMessage>(message) : std::nullopt;
    }
    if (message.type == MessageType::Ack) {
        return message.fields.size() == 1 ? std::optional<ProtocolMessage>(message) : std::nullopt;
    }
    if (message.type == MessageType::Status) {
        return message.fields.size() == 2 ? std::optional<ProtocolMessage>(message) : std::nullopt;
    }
    if (message.type == MessageType::Error) {
        return message.fields.size() == 2 ? std::optional<ProtocolMessage>(message) : std::nullopt;
    }
    if (message.type == MessageType::Cicle) {
        return message.fields.size() == 2 ? std::optional<ProtocolMessage>(message) : std::nullopt;
    }
    if (message.type == MessageType::Load || message.type == MessageType::Unload) {
        return message.fields.size() == 1 ? std::optional<ProtocolMessage>(message) : std::nullopt;
    }
    if (message.type == MessageType::Data) {
        const bool validDataCommand = message.fields.size() == 3;
        const bool validDataTelemetry = message.fields.size() == 3 || message.fields.size() == 5;
        return (validDataCommand || validDataTelemetry) ? std::optional<ProtocolMessage>(message) : std::nullopt;
    }

    return std::nullopt;
}

} // namespace batview::core::protocol
