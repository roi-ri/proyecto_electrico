#pragma once

#include <string>
#include <vector>

namespace batview::core::protocol {

/**
 * @brief Representa un mensaje parseado del protocolo serial.
 */
struct ProtocolMessage {
    std::string type;
    std::vector<std::string> fields;
    std::string raw;
};

} // namespace batview::core::protocol
