#include "ui/frames/MainFrame.h"

#include "ui/frames/MainFrameSupport.h"
#include "ui/panels/PlotPanel.h"
#include "ui/viewmodels/MainViewModel.h"

namespace batview::ui::frames {

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1280, 780)),
      splashTimer_(this),
      wizardStep_(0),
      isConnected_(false),
      batteryChosen_(false),
      functionChosen_(false),
      operationActive_(false),
      isInfiniteRunning_(false),
      connectionInProgress_(false),
      workspaceSplitter_(nullptr),
      workflowSplitter_(nullptr),
      plotPanel_(nullptr),
      trafficLogCtrl_(nullptr) {
    detail::SetAppIcon(*this);
    BuildLayout();
    ConfigureBrandStyle();
    BindEvents();
}

void MainFrame::SetViewModel(std::shared_ptr<batview::ui::viewmodels::MainViewModel> viewModel) {
    viewModel_ = viewModel;

    if (viewModel_) {
        viewModel_->SetTrafficObserver([this](bool outgoing, const std::string& line) {
            AppendTraffic(outgoing, line);
        });
    }

    if (plotPanel_) {
        plotPanel_->SetViewModel(viewModel_);
    }
}

} // namespace batview::ui::frames
