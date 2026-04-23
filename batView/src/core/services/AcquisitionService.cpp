#include "core/services/AcquisitionService.h"

namespace batview::core::services {

AcquisitionService::AcquisitionService(ISerialPort& serialPort,
                                       std::unique_ptr<core::protocol::IMessageParser> messageParser,
                                       ILogger& logger)
        : serialPort_(serialPort),
      messageParser_(std::move(messageParser)),
      logger_(logger) {}

AcquisitionService::~AcquisitionService() {
    StopAcquisition();
}

void AcquisitionService::StartAcquisition() {
    if (isAcquiring_) {
        return;
    }

    if (!serialPort_.IsOpen()) {
        logger_.Error("Cannot start acquisition: serial port is not open.");
        return;
    }

    isAcquiring_ = true;
    acquisitionThread_ = std::thread(&AcquisitionService::AcquisitionLoop, this);
    logger_.Info("Acquisition loop started.");
}

void AcquisitionService::StopAcquisition() {
    if (!isAcquiring_) {
        return;
    }

    isAcquiring_ = false;
    if (acquisitionThread_.joinable()) {
        acquisitionThread_.join();
    }
    logger_.Info("Acquisition loop stopped.");
}

bool AcquisitionService::IsAcquiring() const {
    return isAcquiring_;
}

std::vector<core::models::Measurement> AcquisitionService::GetMeasurements() const {
    std::lock_guard<std::mutex> lock(measurementsMutex_);
    return measurements_;
}

void AcquisitionService::ClearMeasurements() {
    std::lock_guard<std::mutex> lock(measurementsMutex_);
    measurements_.clear();
    logger_.Info("Cleared stored telemetry measurements.");
}

void AcquisitionService::AcquisitionLoop() {
    while (isAcquiring_) {
        std::string line;
        if (serialPort_.ReadLine(line, 250)) {
            ProcessData(line);
        }
    }
}

void AcquisitionService::ProcessData(const std::string& line) {
    auto measurement = messageParser_->ParseLine(line);
    if (measurement.has_value()) {
        std::lock_guard<std::mutex> lock(measurementsMutex_);
        measurements_.push_back(measurement.value());
        return;
    }

    if (line.rfind("#STATUS", 0) == 0 ||
        line.rfind("#ACK", 0) == 0 ||
        line.rfind("#ERROR", 0) == 0) {
        return;
    }

    logger_.Warning("Dropped invalid telemetry frame: " + line);
}

} // namespace batview::core::services
