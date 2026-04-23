#pragma once

#include <string>
#include <vector>

namespace batview::core::protocol {

enum class MessageType {
    Data,
    Cicle,
    Stop,
    Status,
    Ack,
    Error,
    Load,
    Unload,
    Unknown
};

struct ProtocolMessage {
    MessageType type = MessageType::Unknown;
    std::string typeToken;
    std::vector<std::string> fields;
};

struct StatusPayload {
    std::string state;
    std::string detail;
};

struct ErrorPayload {
    std::string code;
    std::string description;
};

} // namespace batview::core::protocol
