#include "ui/viewmodels/MainViewModel.h"

namespace batview::ui::viewmodels {

MainViewModel::MainViewModel(
    std::shared_ptr<batview::core::services::ConnectionService> connectionService,
    std::shared_ptr<batview::core::services::AcquisitionService> acquisitionService,
    std::shared_ptr<batview::core::services::ExportService> exportService,
    std::shared_ptr<batview::core::services::PlotService> plotService)
    : connectionService_(connectionService),
      acquisitionService_(acquisitionService),
      exportService_(exportService),
      plotService_(plotService) {}

void MainViewModel::ConnectToDevice(const std::string& portName) {
    connectionService_->Connect(portName, 115200);
}

void MainViewModel::StartAcquisition() {
    acquisitionService_->StartAcquisition();
}

void MainViewModel::StopAcquisition() {
    acquisitionService_->StopAcquisition();
}

void MainViewModel::ExportData(const std::string& filePath) {
    exportService_->Export(filePath, acquisitionService_->GetMeasurements());
}

void MainViewModel::PlotData() {
    plotService_->Plot(acquisitionService_->GetMeasurements());
}

} // namespace batview::ui::viewmodels
