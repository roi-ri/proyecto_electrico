#include "ui/viewmodels/MainViewModel.h"

#include <chrono>

namespace batview::ui::viewmodels {

MainViewModel::MainViewModel(
    std::shared_ptr<batview::core::services::ConnectionService> connectionService,
    std::shared_ptr<batview::core::services::AcquisitionService> acquisitionService,
        std::shared_ptr<batview::core::services::ProtocolSessionService> protocolSessionService,
    std::shared_ptr<batview::core::services::ExportService> exportService,
    std::shared_ptr<batview::core::services::PlotService> plotService)
    : connectionService_(connectionService),
      acquisitionService_(acquisitionService),
            protocolSessionService_(protocolSessionService),
      exportService_(exportService),
      plotService_(plotService) {
    if (connectionService_) {
        connectionService_->SetTrafficObserver([this](bool outgoing, const std::string& line) {
            if (trafficObserver_) {
                trafficObserver_(outgoing, line);
            }
        });
    }

    if (protocolSessionService_) {
        protocolSessionService_->SetTrafficObserver([this](bool outgoing, const std::string& line) {
            if (trafficObserver_) {
                trafficObserver_(outgoing, line);
            }
        });
    }
}

bool MainViewModel::ConnectToDevice(const std::string& portName) {
    if (connectionService_) {
        connected_ = connectionService_->Connect(portName, 115200);
        return connected_;
    }

    connected_ = false;
    return false;
}

void MainViewModel::DisconnectDevice() {
    if (connectionService_) {
        connectionService_->Disconnect();
    }
    connected_ = false;
}

bool MainViewModel::IsConnected() const {
    return connected_;
}

void MainViewModel::SetTrafficObserver(TrafficObserver observer) {
    trafficObserver_ = std::move(observer);
}

void MainViewModel::StartAcquisition() {
    if (acquisitionService_) {
        acquisitionService_->StartAcquisition();
    }
}

void MainViewModel::StopAcquisition() {
    if (acquisitionService_) {
        acquisitionService_->StopAcquisition();
    }
}

bool MainViewModel::SendBatterySelection(int batteryTypeCode, int functionCode) {
    if (!protocolSessionService_) {
        connected_ = false;
        return false;
    }

    const bool ok = protocolSessionService_->SendBatterySelection(
        batteryTypeCode, functionCode, true, std::chrono::milliseconds(1500));
    connected_ = connected_ && ok;
    return ok;
}

bool MainViewModel::SendCycleConfiguration(bool indefiniteMode, int cycleCount) {
    if (!protocolSessionService_) {
        connected_ = false;
        return false;
    }

    const bool ok = protocolSessionService_->SendCycleConfiguration(
        indefiniteMode, cycleCount, std::chrono::milliseconds(1500));
    connected_ = connected_ && ok;
    return ok;
}

bool MainViewModel::SendStopCommand() {
    if (!protocolSessionService_) {
        connected_ = false;
        return false;
    }

    const bool ok = protocolSessionService_->SendStop(std::chrono::milliseconds(1500));
    connected_ = connected_ && ok;
    return ok;
}

bool MainViewModel::SendLoadTarget(int targetPercent) {
    if (!protocolSessionService_) {
        connected_ = false;
        return false;
    }

    const bool ok = protocolSessionService_->SendLoadTarget(targetPercent, std::chrono::milliseconds(1500));
    connected_ = connected_ && ok;
    return ok;
}

bool MainViewModel::SendUnloadTarget(int targetPercent) {
    if (!protocolSessionService_) {
        connected_ = false;
        return false;
    }

    const bool ok = protocolSessionService_->SendUnloadTarget(targetPercent, std::chrono::milliseconds(1500));
    connected_ = connected_ && ok;
    return ok;
}

std::string MainViewModel::GetLastCommunicationError() const {
    if (protocolSessionService_) {
        const auto& protocolError = protocolSessionService_->GetLastError();
        if (!protocolError.empty()) {
            return protocolError;
        }
    }

    if (connectionService_) {
        return connectionService_->GetLastError();
    }

    return std::string();
}

std::vector<batview::core::models::Measurement> MainViewModel::GetMeasurements() const {
    if (!acquisitionService_) {
        return {};
    }

    return acquisitionService_->GetMeasurements();
}

void MainViewModel::ClearPlotData() {
    if (acquisitionService_) {
        acquisitionService_->ClearMeasurements();
    }
}

batview::core::services::PlotRenderResult MainViewModel::RenderPlot(
    batview::core::services::PlotAxis xAxis,
    batview::core::services::PlotAxis yAxis,
    int width,
    int height,
    const std::string& title) const {
    if (!plotService_ || !acquisitionService_) {
        return {};
    }

    return plotService_->RenderPlot(acquisitionService_->GetMeasurements(), xAxis, yAxis, width, height, title);
}

bool MainViewModel::ExportPlotData(const std::string& filePath,
                                   const std::string& format,
                                   batview::core::services::PlotAxis xAxis,
                                   batview::core::services::PlotAxis yAxis,
                                   std::string& outError) const {
    if (!plotService_ || !acquisitionService_) {
        outError = "No hay datos de gráfica disponibles.";
        return false;
    }

    const auto measurements = acquisitionService_->GetMeasurements();

    if (format == "CSV") {
        return plotService_->ExportPlotCsv(filePath, measurements, xAxis, yAxis, outError);
    }
    if (format == "MAT") {
        return plotService_->ExportPlotMat(filePath, measurements, xAxis, yAxis, outError);
    }
    if (format == "XLSX") {
        return plotService_->ExportPlotXlsx(filePath, measurements, xAxis, yAxis, outError);
    }

    outError = "Formato de exportación no soportado para gráfica.";
    return false;
}

bool MainViewModel::ExportData(const std::string& filePath, const std::string& format, std::string& outError) {
    if (!exportService_ || !acquisitionService_) {
        outError = "No hay datos de sesión disponibles para exportar.";
        return false;
    }

    const auto measurements = acquisitionService_->GetMeasurements();
    if (measurements.empty()) {
        outError = "No hay mediciones capturadas para exportar.";
        return false;
    }

    if (format == "CSV") {
        return exportService_->ExportToCsv(filePath, measurements, outError);
    }
    if (format == "MAT") {
        return exportService_->ExportToMat(filePath, measurements, outError);
    }
    if (format == "XLSX") {
        return exportService_->ExportToXlsx(filePath, measurements, outError);
    }

    outError = "Formato de exportación de sesión no soportado.";
    return false;
}

void MainViewModel::UpdatePlot() {
    if (plotService_ && acquisitionService_) {
        (void)plotService_->RenderPlot(acquisitionService_->GetMeasurements(),
                                       batview::core::services::PlotAxis::TimestampMs,
                                       batview::core::services::PlotAxis::Voltage,
                                       640,
                                       320,
                                       "Preview");
    }
}

} // namespace batview::ui::viewmodels
