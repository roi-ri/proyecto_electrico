#pragma once

#include <string>

namespace batview::core::protocol {

/**
 * @brief Clase responsable de validar la estructura de los mensajes recibidos.
 */
class ProtocolValidator {
public:
    bool Validate(const std::string& line) const;
};

} // namespace batview::core::protocol
