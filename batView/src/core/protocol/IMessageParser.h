#pragma once

#include <optional>
#include <string>
#include "ProtocolMessage.h"

namespace batview::core::protocol {

/**
 * @brief Interfaz para parseo de mensajes entrantes.
 */
class IMessageParser {
public:
    virtual ~IMessageParser() = default;
    virtual std::optional<ProtocolMessage> ParseLine(const std::string& line) const = 0;
};

} // namespace batview::core::protocol
