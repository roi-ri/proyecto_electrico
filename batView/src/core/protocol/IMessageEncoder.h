#pragma once

#include <string>

namespace batview::core::protocol {

/**
 * @brief Interfaz para generar mensajes salientes según el protocolo.
 */
class IMessageEncoder {
public:
    virtual ~IMessageEncoder() = default;

    virtual std::string EncodeSelection(int batterySelected, int batteryType, int function) const = 0;
    virtual std::string EncodeCycleCommand(int cycleType, int cycleCount) const = 0;
    virtual std::string EncodeStopCommand() const = 0;
    virtual std::string EncodeLoadCommand(int targetPercent) const = 0;
    virtual std::string EncodeUnloadCommand(int targetPercent) const = 0;
};

} // namespace batview::core::protocol
