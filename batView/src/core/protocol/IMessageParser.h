#pragma once

#include <optional>
#include <string>

#include "models/Measurement.h"

namespace batview::core::protocol {

class IMessageParser {
public:
    virtual ~IMessageParser() = default;

    virtual std::optional<core::models::Measurement> ParseLine(const std::string& line) const = 0;
};

} // namespace batview::core::protocol
