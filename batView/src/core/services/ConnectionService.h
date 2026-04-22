#pragma once

#include <memory>
#include <string>
#include "core/protocol/IMessageEncoder.h"
#include "ISerialPort.h"
#include "ILogger.h"

namespace batview::core::services {

/**
 * @brief Servicio encargado de gestionar la conexión con el ESP32.
 *
 * Su responsabilidad es encapsular la apertura/cierre del puerto y
 * el envío de mensajes de alto nivel ya codificados por el protocolo.
 */
class ConnectionService {
public:
    ConnectionService(std::unique_ptr<ISerialPort> serialPort,
                      std::unique_ptr<batview::core::protocol::IMessageEncoder> encoder,
                      ILogger& logger);

    bool Connect(const std::string& portName, int baudRate);
    void Disconnect();
    bool IsConnected() const;

    bool SendRaw(const std::string& message);
    bool SendSelection(int batterySelected, int batteryType, int function);
    bool SendCycleCommand(int cycleType, int cycleCount);
    bool SendStop();
    bool SendLoadCommand(int targetPercent);
    bool SendUnloadCommand(int targetPercent);

private:
    std::unique_ptr<ISerialPort> serialPort_;
    std::unique_ptr<batview::core::protocol::IMessageEncoder> encoder_;
    ILogger& logger_;
};

} // namespace batview::core::services
