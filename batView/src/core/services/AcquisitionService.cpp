#include "core/services/AcquisitionService.h"
#include <thread>
#include <chrono>

namespace batview::core::services {

AcquisitionService::AcquisitionService(std::unique_ptr<ISerialPort> serialPort,
                                       std::unique_ptr<IMessageParser> messageParser,
                                       ILogger& logger)
    : serialPort_(std::move(serialPort)),
      messageParser_(std::move(messageParser)),
      logger_(logger) {}

void AcquisitionService::StartAcquisition() {
    if (!serialPort_->IsOpen()) {
        logger_.Error("Puerto serial no abierto.");
        return;
    }

    isAcquiring_ = true;
    std::thread([this]() {
        while (isAcquiring_) {
            std::string line;
            if (serialPort_->ReadLine(line, 1000)) {
                ProcessData(line);
            }
        }
    }).detach();
}

void AcquisitionService::StopAcquisition() {
    isAcquiring_ = false;
}

bool AcquisitionService::IsAcquiring() const {
    return isAcquiring_;
}

void AcquisitionService::ProcessData(const std::string& line) {
    auto message = messageParser_->ParseLine(line);
    if (message) {
        // Aquí procesamos el mensaje, por ejemplo, creando un objeto Measurement
        core::models::Measurement measurement;
        // (Poblar measurement con datos del mensaje)
        measurements_.push_back(measurement);
    } else {
        logger_.Warning("Mensaje no válido recibido: " + line);
    }
}

} // namespace batview::core::services
