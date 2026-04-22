#pragma once

#include <memory>
#include "core/services/ConnectionService.h"
#include "core/services/AcquisitionService.h"
#include "core/services/ExportService.h"
#include "core/services/PlotService.h"

namespace batview::ui::viewmodels {

/**
 * @brief ViewModel que coordina la lógica entre la UI y los servicios del core.
 */
class MainViewModel {
public:
    MainViewModel(std::shared_ptr<batview::core::services::ConnectionService> connectionService,
                  std::shared_ptr<batview::core::services::AcquisitionService> acquisitionService,
                  std::shared_ptr<batview::core::services::ExportService> exportService,
                  std::shared_ptr<batview::core::services::PlotService> plotService);

    void ConnectToDevice(const std::string& portName);
    void StartAcquisition();
    void StopAcquisition();
    void ExportData(const std::string& filePath);
    void PlotData();

private:
    std::shared_ptr<batview::core::services::ConnectionService> connectionService_;
    std::shared_ptr<batview::core::services::AcquisitionService> acquisitionService_;
    std::shared_ptr<batview::core::services::ExportService> exportService_;
    std::shared_ptr<batview::core::services::PlotService> plotService_;
};

} // namespace batview::ui::viewmodels
