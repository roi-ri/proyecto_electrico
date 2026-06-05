#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "core/protocol/IMessageParser.h"
#include "core/services/ILogger.h"
#include "core/services/ISerialPort.h"
#include "models/Measurement.h"

namespace batview::core::services {

class AcquisitionService {
public:
    AcquisitionService(ISerialPort& serialPort,
                       std::unique_ptr<core::protocol::IMessageParser> messageParser,
                       ILogger& logger);
    ~AcquisitionService();

    void StartAcquisition();
    void StopAcquisition();
    bool IsAcquiring() const;
    std::vector<core::models::Measurement> GetMeasurements() const;
    void ClearMeasurements();

private:
    void AcquisitionLoop();
    void ProcessData(const std::string& line);

    ISerialPort& serialPort_;
    std::unique_ptr<core::protocol::IMessageParser> messageParser_;
    ILogger& logger_;
    std::atomic<bool> isAcquiring_{false};
    std::thread acquisitionThread_;
    mutable std::mutex measurementsMutex_;
    std::vector<core::models::Measurement> measurements_;
};

} // namespace batview::core::services
