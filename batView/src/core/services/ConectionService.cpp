#include "core/services/ConnectionService.h"

namespace batview::core::services {

ConnectionService::ConnectionService(
    std::unique_ptr<ISerialPort> serialPort,
    std::unique_ptr<batview::core::protocol::IMessageEncoder> encoder,
    ILogger& logger)
    : serialPort_(std::move(serialPort)),
      encoder_(std::move(encoder)),
      logger_(logger) {}

bool ConnectionService::Connect(const std::string& portName, int baudRate) {
    logger_.Info("Intentando conexión serial con: " + portName);
    return serialPort_->Open(portName, baudRate);
}

void ConnectionService::Disconnect() {
    logger_.Info("Cerrando conexión serial.");
    serialPort_->Close();
}

bool ConnectionService::IsConnected() const {
    return serialPort_->IsOpen();
}

bool ConnectionService::SendRaw(const std::string& message) {
    if (!serialPort_->IsOpen()) {
        logger_.Error("No se puede enviar mensaje: puerto serial cerrado.");
        return false;
    }
    return serialPort_->WriteLine(message);
}

bool ConnectionService::SendSelection(int batterySelected, int batteryType, int function) {
    return SendRaw(encoder_->EncodeSelection(batterySelected, batteryType, function));
}

bool ConnectionService::SendCycleCommand(int cycleType, int cycleCount) {
    return SendRaw(encoder_->EncodeCycleCommand(cycleType, cycleCount));
}

bool ConnectionService::SendStop() {
    return SendRaw(encoder_->EncodeStopCommand());
}

bool ConnectionService::SendLoadCommand(int targetPercent) {
    return SendRaw(encoder_->EncodeLoadCommand(targetPercent));
}

bool ConnectionService::SendUnloadCommand(int targetPercent) {
    return SendRaw(encoder_->EncodeUnloadCommand(targetPercent));
}

} // namespace batview::core::services
