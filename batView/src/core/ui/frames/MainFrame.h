#pragma once

#include <wx/frame.h>
#include "ui/panels/ConnectionPanel.h"
#include "ui/panels/PlotPanel.h"
#include "ui/viewmodels/MainViewModel.h"

namespace batview::ui::frames {

class MainFrame : public wxFrame {
public:
    explicit MainFrame(const wxString& title);

    void SetViewModel(std::shared_ptr<batview::ui::viewmodels::MainViewModel> viewModel);

private:
    void BuildLayout();
    void OnConnectButton(wxCommandEvent& event);
    void OnStartAcquisition(wxCommandEvent& event);
    void OnStopAcquisition(wxCommandEvent& event);

    wxPanel* mainPanel_;
    ConnectionPanel* connectionPanel_;
    PlotPanel* plotPanel_;
    std::shared_ptr<batview::ui::viewmodels::MainViewModel> viewModel_;
};

} // namespace batview::ui::frames
