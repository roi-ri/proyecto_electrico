#include "core/protocol/TextMessageParser.h"

#include <sstream>
#include <string>
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
} // namespace

std::optional<core::models::Measurement> TextMessageParser::ParseLine(const std::string& line) const {
    const auto parts = Split(line, ',');
    if (parts.empty() || parts[0] != "#DATA") {
        return std::nullopt;
    }

    core::models::Measurement measurement;

    try {
        if (parts.size() == 4) {
            measurement.voltage = std::stod(parts[1]);
            measurement.current = std::stod(parts[2]);
            measurement.timestampMs = static_cast<std::uint64_t>(std::stoull(parts[3]));
            return measurement;
        }

        if (parts.size() == 6) {
            measurement.voltage = std::stod(parts[1]);
            measurement.current = std::stod(parts[2]);
            measurement.timestampMs = static_cast<std::uint64_t>(std::stoull(parts[3]));
            measurement.state = std::stoi(parts[4]);
            measurement.completedCycles = std::stoi(parts[5]);
            return measurement;
        }
    } catch (...) {
        return std::nullopt;
    }

    return std::nullopt;
}

} // namespace batview::core::protocol
