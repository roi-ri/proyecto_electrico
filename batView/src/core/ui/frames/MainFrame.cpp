#include "ui/frames/MainFrame.h"
#include <wx/msgdlg.h>

namespace batview::ui::frames {

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1000, 700)) {
    BuildLayout();
}

void MainFrame::SetViewModel(std::shared_ptr<batview::ui::viewmodels::MainViewModel> viewModel) {
    viewModel_ = viewModel;
    connectionPanel_->BindConnectionButton(&MainFrame::OnConnectButton, this);
}

void MainFrame::BuildLayout() {
    mainPanel_ = new wxPanel(this);
    connectionPanel_ = new ConnectionPanel(mainPanel_);
    plotPanel_ = new PlotPanel(mainPanel_);

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(connectionPanel_, 0, wxEXPAND | wxALL, 10);
    sizer->Add(plotPanel_, 1, wxEXPAND | wxALL, 10);
    mainPanel_->SetSizer(sizer);
}

void MainFrame::OnConnectButton(wxCommandEvent& event) {
    // Lógica para establecer la conexión
    std::string portName = "/dev/ttyUSB0"; // Ejemplo de puerto
    viewModel_->ConnectToDevice(portName);
    connectionPanel_->SetConnectionStatus(true); // Simulamos una conexión exitosa
}

void MainFrame::OnStartAcquisition(wxCommandEvent& event) {
    viewModel_->StartAcquisition();
}

void MainFrame::OnStopAcquisition(wxCommandEvent& event) {
    viewModel_->StopAcquisition();
}

} // namespace batview::ui::frames
