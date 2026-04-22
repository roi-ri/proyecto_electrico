#pragma once

#include <vector>
#include <string>

namespace batview::infrastructure::serial {

/**
 * @brief Servicio encargado de enumerar los puertos seriales disponibles.
 */
class SerialEnumerator {
public:
    static std::vector<std::string> EnumeratePorts();
};

} // namespace batview::infrastructure::serial
