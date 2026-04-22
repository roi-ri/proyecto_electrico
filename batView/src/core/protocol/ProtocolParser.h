#pragma once

#include "IMessageParser.h"

namespace batview::core::protocol {

/**
 * @brief Implementación base del parser del protocolo.
 */
class ProtocolParser : public IMessageParser {
public:
    std::optional<ProtocolMessage> ParseLine(const std::string& line) const override;
};

} // namespace batview::core::protocol
