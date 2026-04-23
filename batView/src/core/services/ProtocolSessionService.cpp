#include "core/services/ProtocolSessionService.h"

#include <exception>

namespace batview::core::services {

ProtocolSessionService::ProtocolSessionService(ISerialPort& serialPort,
                                               core::protocol::IProtocolCodec& protocolCodec,
                                               ILogger& logger)
    : serialPort_(serialPort),
      protocolCodec_(protocolCodec),
      logger_(logger) {}

bool ProtocolSessionService::SendBatterySelection(int batteryTypeCode,
                                                  int functionCode,
                                                  bool batterySelected,
                                                  std::chrono::milliseconds timeout) {
    try {
        return SendCommandAndAwaitAck(
            protocolCodec_.BuildSelectBatteryCommand(batterySelected, batteryTypeCode, functionCode),
            "DATA", timeout);
    } catch (const std::exception& ex) {
        lastError_ = std::string("Comando invalido de seleccion de bateria: ") + ex.what();
        logger_.Error(lastError_);
        return false;
    }
}

bool ProtocolSessionService::SendCycleConfiguration(bool indefiniteMode,
                                                    int cycleCount,
                                                    std::chrono::milliseconds timeout) {
    try {
        return SendCommandAndAwaitAck(protocolCodec_.BuildCycleCommand(indefiniteMode, cycleCount),
                                      "CICLE", timeout);
    } catch (const std::exception& ex) {
        lastError_ = std::string("Comando invalido de ciclo: ") + ex.what();
        logger_.Error(lastError_);
        return false;
    }
}

bool ProtocolSessionService::SendStop(std::chrono::milliseconds timeout) {
    return SendCommandAndAwaitAck(protocolCodec_.BuildStopCommand(), "STOP", timeout);
}

bool ProtocolSessionService::SendLoadTarget(int targetPercent, std::chrono::milliseconds timeout) {
    try {
        return SendCommandAndAwaitAck(protocolCodec_.BuildLoadCommand(targetPercent),
                                      "LOAD", timeout);
    } catch (const std::exception& ex) {
        lastError_ = std::string("Comando invalido de carga: ") + ex.what();
        logger_.Error(lastError_);
        return false;
    }
}

bool ProtocolSessionService::SendUnloadTarget(int targetPercent, std::chrono::milliseconds timeout) {
    try {
        return SendCommandAndAwaitAck(protocolCodec_.BuildUnloadCommand(targetPercent),
                                      "UNLOAD", timeout);
    } catch (const std::exception& ex) {
        lastError_ = std::string("Comando invalido de descarga: ") + ex.what();
        logger_.Error(lastError_);
        return false;
    }
}

void ProtocolSessionService::SetTrafficObserver(TrafficObserver observer) {
    trafficObserver_ = std::move(observer);
}

const std::string& ProtocolSessionService::GetLastError() const {
    return lastError_;
}

bool ProtocolSessionService::SendCommandAndAwaitAck(const std::string& command,
                                                    const std::string& expectedAckToken,
                                                    std::chrono::milliseconds timeout) {
    lastError_.clear();

    if (!serialPort_.IsOpen()) {
        lastError_ = "No se puede enviar el comando porque el puerto serial esta cerrado.";
        logger_.Error(lastError_);
        return false;
    }

    if (!serialPort_.WriteLine(command)) {
        lastError_ = "No se pudo escribir el comando al puerto serial: " + command;
        logger_.Error(lastError_);
        return false;
    }

    if (trafficObserver_) {
        trafficObserver_(true, command);
    }

    const auto started = std::chrono::steady_clock::now();

    while (std::chrono::steady_clock::now() - started < timeout) {
        std::string line;
        if (!serialPort_.ReadLine(line, 100)) {
            continue;
        }

        if (trafficObserver_) {
            trafficObserver_(false, line);
        }

        const auto parsed = protocolCodec_.ParseMessage(line);
        if (!parsed.has_value()) {
            logger_.Warning("Received invalid protocol frame: " + line);
            continue;
        }

        if (parsed->type == core::protocol::MessageType::Ack) {
            if (parsed->fields[0] == expectedAckToken) {
                lastError_.clear();
                logger_.Info("ACK received for command " + expectedAckToken);
                return true;
            }

            logger_.Warning("Unexpected ACK token: " + parsed->fields[0]);
            continue;
        }

        if (parsed->type == core::protocol::MessageType::Error) {
            lastError_ = "El ESP32 respondio con error " + parsed->fields[0] + ": " + parsed->fields[1];
            logger_.Error(lastError_);
            return false;
        }

        if (parsed->type == core::protocol::MessageType::Status) {
            logger_.Info("Device status: " + parsed->fields[0] + " - " + parsed->fields[1]);
            continue;
        }
    }

    lastError_ = "Se envio " + command + ", pero no llego #ACK," + expectedAckToken + " antes del timeout.";
    logger_.Error(lastError_);
    return false;
}

} // namespace batview::core::services
