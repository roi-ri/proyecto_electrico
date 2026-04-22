#include "core/protocol/ProtocolValidator.h"
#include <regex>

namespace batview::core::protocol {

bool ProtocolValidator::Validate(const std::string& line) const {
    // Ejemplo de validación básica usando una expresión regular para verificar formato
    std::regex messagePattern("^#([A-Z]+),([0-9,]+)$");
    return std::regex_match(line, messagePattern);
}

} // namespace batview::core::protocol
