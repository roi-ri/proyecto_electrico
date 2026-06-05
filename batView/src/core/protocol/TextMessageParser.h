#pragma once

#include "core/protocol/IMessageParser.h"

namespace batview::core::protocol {

class TextMessageParser : public IMessageParser {
public:
    std::optional<core::models::Measurement> ParseLine(const std::string& line) const override;
};

} // namespace batview::core::protocol
