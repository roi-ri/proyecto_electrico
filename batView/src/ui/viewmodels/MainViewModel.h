#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "core/services/AcquisitionService.h"
#include "core/services/ConnectionService.h"
#include "core/services/ExportService.h"
#include "core/services/PlotService.h"
#include "core/services/ProtocolSessionService.h"

namespace batview::ui::viewmodels {

class MainViewModel {
public:
    using TrafficObserver = std::function<void(bool, const std::string&)>;

    MainViewModel(std::shared_ptr<batview::core::services::ConnectionService> connectionService,
                  std::shared_ptr<batview::core::services::AcquisitionService> acquisitionService,
                  std::shared_ptr<batview::core::services::ProtocolSessionService> protocolSessionService,
                  std::shared_ptr<batview::core::services::ExportService> exportService,
                  std::shared_ptr<batview::core::services::PlotService> plotService);

    bool ConnectToDevice(const std::string& portName);
    void DisconnectDevice();
    bool IsConnected() const;
    void SetTrafficObserver(TrafficObserver observer);
    void StartAcquisition();
    void StopAcquisition();
    bool SendBatterySelection(int batteryTypeCode, int functionCode);
    bool SendCycleConfiguration(bool indefiniteMode, int cycleCount);
    bool SendStopCommand();
    bool SendLoadTarget(int targetPercent);
    bool SendUnloadTarget(int targetPercent);
    std::string GetLastCommunicationError() const;
    std::vector<batview::core::models::Measurement> GetMeasurements() const;
    void ClearPlotData();
    batview::core::services::PlotRenderResult RenderPlot(batview::core::services::PlotAxis xAxis,
                                                         batview::core::services::PlotAxis yAxis,
                                                         int width,
                                                         int height,
                                                         const std::string& title) const;
    bool ExportPlotData(const std::string& filePath,
                        const std::string& format,
                        batview::core::services::PlotAxis xAxis,
                        batview::core::services::PlotAxis yAxis,
                        std::string& outError) const;
    bool ExportData(const std::string& filePath, const std::string& format, std::string& outError);
    void UpdatePlot();

private:
    std::shared_ptr<batview::core::services::ConnectionService> connectionService_;
    std::shared_ptr<batview::core::services::AcquisitionService> acquisitionService_;
    std::shared_ptr<batview::core::services::ProtocolSessionService> protocolSessionService_;
    std::shared_ptr<batview::core::services::ExportService> exportService_;
    std::shared_ptr<batview::core::services::PlotService> plotService_;
    TrafficObserver trafficObserver_;
    bool connected_ = false;
};

} // namespace batview::ui::viewmodels
