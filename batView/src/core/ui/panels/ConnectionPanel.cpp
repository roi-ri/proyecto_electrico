#include "ui/panels/ConnectionPanel.h"
#include <wx/sizer.h>

namespace batview::ui::panels {

ConnectionPanel::ConnectionPanel(wxWindow* parent)
    : wxPanel(parent) {

    statusText_ = new wxStaticText(this, wxID_ANY, "Desconectado");
    connectButton_ = new wxButton(this, wxID_ANY, "Conectar");

    auto* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(statusText_, 0, wxALL, 5);
    sizer->Add(connectButton_, 0, wxALL, 5);
    SetSizer(sizer);
}

void ConnectionPanel::SetConnectionStatus(bool connected) {
    statusText_->SetLabel(connected ? "Conectado" : "Desconectado");
}

void ConnectionPanel::BindConnectionButton(wxCommandEventHandler handler) {
    connectButton_->Bind(wxEVT_BUTTON, handler);
}

} // namespace batview::ui::panels
