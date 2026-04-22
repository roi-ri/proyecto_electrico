#include "core/protocol/ProtocolParser.h"
#include <sstream>

namespace batview::core::protocol {

std::optional<ProtocolMessage> ProtocolParser::ParseLine(const std::string& line) const {
    if (line.empty() || line[0] != '#') {
        return std::nullopt;
    }

    ProtocolMessage message;
    message.raw = line;

    std::string content = line.substr(1);
    std::stringstream ss(content);
    std::string token;

    bool first = true;
    while (std::getline(ss, token, ',')) {
        if (first) {
            message.type = token;
            first = false;
        } else {
            if (!token.empty() && token.back() == '\n') {
                token.pop_back();
            }
            if (!token.empty() && token.back() == '\r') {
                token.pop_back();
            }
            message.fields.push_back(token);
        }
    }

    return message;
}

} // namespace batview::core::protocol
