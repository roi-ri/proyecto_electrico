#include "ui/frames/MainFrame.h"
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>

namespace batview::ui::frames {

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1000, 700)) {
    BuildLayout();
}

void MainFrame::BuildLayout() {
    auto* panel = new wxPanel(this);
    auto* sizer = new wxBoxSizer(wxVERTICAL);

    auto* title = new wxStaticText(panel, wxID_ANY, "batView - Base estructural");
    auto* status = new wxStaticText(panel, wxID_ANY, "Estado: desconectado");
    auto* connectButton = new wxButton(panel, wxID_ANY, "Conectar ESP32");

    sizer->Add(title, 0, wxALL, 10);
    sizer->Add(status, 0, wxALL, 10);
    sizer->Add(connectButton, 0, wxALL, 10);

    panel->SetSizer(sizer);
}

} // namespace batview::ui::frames
