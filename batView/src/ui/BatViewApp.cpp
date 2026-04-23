#include "ui/BatViewApp.h"

#ifdef BATVIEW_ENABLE_WX

#include <memory>

#include <wx/image.h>

#include "core/protocol/TextProtocolCodec.h"
#include "core/protocol/TextMessageParser.h"
#include "core/services/AcquisitionService.h"
#include "core/services/ConnectionService.h"
#include "core/services/ExportService.h"
#include "core/services/PlotService.h"
#include "core/services/ProtocolSessionService.h"
#include "infrastructure/csv/CsvExporter.h"
#include "infrastructure/logging/ConsoleLogger.h"
#include "infrastructure/serial/NativeSerialPort.h"
#include "ui/frames/MainFrame.h"
#include "ui/viewmodels/MainViewModel.h"

namespace batview::ui {

bool BatViewApp::OnInit() {
    wxInitAllImageHandlers();

    auto* logger = new batview::infrastructure::logging::ConsoleLogger();
    auto* csvExporter = new batview::infrastructure::csv::CsvExporter();
    auto* serialPort = new batview::infrastructure::serial::NativeSerialPort();
    auto* parser = new batview::core::protocol::TextMessageParser();
    auto* protocolCodec = new batview::core::protocol::TextProtocolCodec();

    auto connectionService = std::make_shared<batview::core::services::ConnectionService>(*serialPort, *logger);
    auto acquisitionService = std::make_shared<batview::core::services::AcquisitionService>(*serialPort, std::unique_ptr<batview::core::protocol::IMessageParser>(parser), *logger);
    auto protocolSessionService = std::make_shared<batview::core::services::ProtocolSessionService>(*serialPort, *protocolCodec, *logger);
    auto exportService = std::make_shared<batview::core::services::ExportService>(*csvExporter);
    auto plotService = std::make_shared<batview::core::services::PlotService>();

    auto viewModel = std::make_shared<batview::ui::viewmodels::MainViewModel>(
        connectionService, acquisitionService, protocolSessionService, exportService, plotService);

    auto* frame = new batview::ui::frames::MainFrame("batView");
    frame->SetViewModel(viewModel);
    frame->Show(true);
    SetTopWindow(frame);
    return true;
}

} // namespace batview::ui

wxIMPLEMENT_APP(batview::ui::BatViewApp);

#endif
