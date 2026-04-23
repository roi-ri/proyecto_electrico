#ifdef BATVIEW_ENABLE_WX

#include "ui/BatViewApp.h"

#else

#include <memory>

#include "app/AppController.h"
#include "core/protocol/TextProtocolCodec.h"
#include "core/protocol/TextMessageParser.h"
#include "core/services/AcquisitionService.h"
#include "core/services/ConnectionService.h"
#include "core/services/ExportService.h"
#include "core/services/PlotService.h"
#include "core/services/ProtocolSessionService.h"
#include "infrastructure/csv/CsvExporter.h"
#include "infrastructure/logging/ConsoleLogger.h"
#include "infrastructure/serial/SerialPortStub.h"
#include "ui/viewmodels/MainViewModel.h"

int main() {
    using namespace batview;

    infrastructure::logging::ConsoleLogger logger;
    infrastructure::csv::CsvExporter csvExporter;
    infrastructure::serial::SerialPortStub serialPort;

    auto parser = std::make_unique<core::protocol::TextMessageParser>();
    core::protocol::TextProtocolCodec protocolCodec;

    auto connectionService = std::make_shared<core::services::ConnectionService>(serialPort, logger);
    auto acquisitionService = std::make_shared<core::services::AcquisitionService>(serialPort, std::move(parser), logger);
    auto protocolSessionService = std::make_shared<core::services::ProtocolSessionService>(
        serialPort, protocolCodec, logger);
    auto exportService = std::make_shared<core::services::ExportService>(csvExporter);
    auto plotService = std::make_shared<core::services::PlotService>();

    ui::viewmodels::MainViewModel viewModel(
        connectionService, acquisitionService, protocolSessionService, exportService, plotService);
    app::AppController controller(viewModel);

    controller.Connect("/dev/tty.usbserial-stub");
    controller.SelectBatteryForFunction(2, 1);
    controller.StartCycle(true, 0);
    controller.StopCycle();
    std::string exportError;
    (void)viewModel.ExportData("data/session_stub.csv", "CSV", exportError);

    logger.Info("batView base architecture initialized.");
    return 0;
}

#endif
