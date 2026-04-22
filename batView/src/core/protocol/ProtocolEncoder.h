#pragma once

#include "IMessageEncoder.h"

namespace batview::core::protocol {

/**
 * @brief Implementación concreta del generador de mensajes del protocolo.
 */
class ProtocolEncoder : public IMessageEncoder {
public:
    std::string EncodeSelection(int batterySelected, int batteryType, int function) const override;
    std::string EncodeCycleCommand(int cycleType, int cycleCount) const override;
    std::string EncodeStopCommand() const override;
    std::string EncodeLoadCommand(int targetPercent) const override;
    std::string EncodeUnloadCommand(int targetPercent) const override;
};

} // namespace batview::core::protocol
