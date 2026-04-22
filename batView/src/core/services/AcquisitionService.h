#pragma once

#include <vector>
#include "core/models/Measurement.h"
#include "ISerialPort.h"
#include "ILogger.h"
#include "IMessageParser.h"

namespace batview::core::services {

/**
 * @brief Servicio encargado de gestionar la adquisición de datos.
 *
 * Su responsabilidad es recibir datos del puerto serial, parsearlos,
 * transformarlos en unidades de medida y almacenarlos en memoria.
 */
class AcquisitionService {
public:
    AcquisitionService(std::unique_ptr<ISerialPort> serialPort,
                       std::unique_ptr<IMessageParser> messageParser,
                       ILogger& logger);

    void StartAcquisition();
    void StopAcquisition();
    bool IsAcquiring() const;

private:
    void ReadSerialData();
    void ProcessData(const std::string& line);

    std::unique_ptr<ISerialPort> serialPort_;
    std::unique_ptr<IMessageParser> messageParser_;
    ILogger& logger_;
    bool isAcquiring_ = false;
    std::vector<core::models::Measurement> measurements_;
};

} // namespace batview::core::services
