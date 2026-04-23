#include "ui/panels/ConnectionPanel.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <wx/font.h>
#include <wx/sizer.h>

#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace batview::ui::panels {

namespace {

std::string TrimAscii(std::string value) {
    auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

std::vector<std::string> DiscoverSerialPorts() {
    std::vector<std::string> ports;

#if defined(_WIN32)
    char targetPath[512];
    for (int index = 1; index <= 256; ++index) {
        const std::string portName = "COM" + std::to_string(index);
        const DWORD result = QueryDosDeviceA(portName.c_str(), targetPath, static_cast<DWORD>(sizeof(targetPath)));
        if (result != 0) {
            ports.push_back(portName);
        }
    }
#elif defined(__APPLE__) || defined(__linux__)
    const std::filesystem::path devDir("/dev");
    if (!std::filesystem::exists(devDir)) {
        return ports;
    }

    for (const auto& entry : std::filesystem::directory_iterator(devDir)) {
        if (!entry.is_character_file() && !entry.is_regular_file()) {
            continue;
        }

        const std::string name = entry.path().filename().string();
        if (name.rfind("tty.", 0) == 0 || name.rfind("cu.", 0) == 0 ||
            name.rfind("ttyUSB", 0) == 0 || name.rfind("ttyACM", 0) == 0 ||
            name.rfind("ttyS", 0) == 0 || name.rfind("rfcomm", 0) == 0) {
#if defined(__APPLE__)
            if (name.rfind("cu.", 0) != 0) {
                continue;
            }
#endif
            ports.push_back(entry.path().string());
        }
    }

    std::sort(ports.begin(), ports.end());
#endif

    return ports;
}

} // namespace

ConnectionPanel::ConnectionPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE) {
    SetBackgroundStyle(wxBG_STYLE_SYSTEM);

    auto* titleText = new wxStaticText(this, wxID_ANY, "Centro de Control");
    wxFont titleFont = titleText->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 3);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleText->SetFont(titleFont);

    statusText_ = new wxStaticText(this, wxID_ANY, "desconectado");

    auto* portLabel = new wxStaticText(this, wxID_ANY, "Puerto serial");
    wxArrayString emptyPorts;
    portCtrl_ = new wxComboBox(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, emptyPorts, wxCB_DROPDOWN);
    portCtrl_->SetHint("Selecciona el puerto del ESP32");

    connectButton_ = new wxButton(this, wxID_ANY, "Conectar");
    connectButton_->SetMinSize(wxSize(180, 52));

    RefreshPorts();

    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(titleText, 0, wxLEFT | wxTOP, 4);
    sizer->Add(statusText_, 0, wxLEFT | wxTOP | wxBOTTOM, 4);
    sizer->Add(portLabel, 0, wxLEFT | wxTOP, 4);
    sizer->Add(portCtrl_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 6);
    sizer->Add(connectButton_, 0, wxTOP | wxBOTTOM, 8);
    SetSizer(sizer);
    Layout();
    SetMinSize(GetBestSize());

    SetDisconnected();
}

void ConnectionPanel::ApplyStatusStyle(const wxColour& color) {
    wxFont statusFont = statusText_->GetFont();
    statusFont.SetPointSize(statusFont.GetPointSize() + 1);
    statusFont.SetWeight(wxFONTWEIGHT_BOLD);
    statusText_->SetFont(statusFont);
    statusText_->SetForegroundColour(color);
}

void ConnectionPanel::RefreshPorts() {
    if (!portCtrl_) {
        return;
    }

    const wxString currentValue = portCtrl_->GetValue();
    portCtrl_->Clear();

    const auto ports = DiscoverSerialPorts();
    for (const auto& port : ports) {
        portCtrl_->Append(port);
    }

    if (!currentValue.empty()) {
        portCtrl_->SetValue(currentValue);
    }

    if (!ports.empty()) {
        const int selection = portCtrl_->FindString(currentValue);
        if (selection != wxNOT_FOUND) {
            portCtrl_->SetSelection(selection);
        } else if (portCtrl_->GetSelection() == wxNOT_FOUND) {
            portCtrl_->SetSelection(0);
            portCtrl_->SetValue(portCtrl_->GetStringSelection());
        }
        return;
    }

    if (currentValue.empty()) {
#if defined(_WIN32)
        portCtrl_->SetValue("COM3");
#elif defined(__APPLE__)
        portCtrl_->SetValue("/dev/cu.usbmodem101");
#else
        portCtrl_->SetValue("/dev/ttyUSB0");
#endif
    }
}

void ConnectionPanel::SetStatusText(const wxString& text, const wxString& tooltip) {
    statusText_->SetLabel(text);
    statusText_->SetToolTip(tooltip.empty() ? text : tooltip);
}

void ConnectionPanel::SetDisconnected() {
    SetStatusText("desconectado");
    ApplyStatusStyle(wxColour(150, 65, 65));
    connectButton_->SetLabel("Conectar");
    connectButton_->Enable(true);
    portCtrl_->Enable(true);
}

void ConnectionPanel::SetConnecting() {
    SetStatusText("conectando...");
    ApplyStatusStyle(wxColour(242, 242, 17));
    connectButton_->SetLabel("Conectando...");
    connectButton_->Enable(false);
    portCtrl_->Enable(false);
}

void ConnectionPanel::SetConnected() {
    SetStatusText("conectado", "Conexion confirmada con ACK del ESP32.");
    ApplyStatusStyle(wxColour(37, 245, 24));
    connectButton_->SetLabel("Reconectar");
    connectButton_->Enable(true);
    portCtrl_->Enable(true);
}

void ConnectionPanel::SetConnectionError(const std::string& detail) {
    SetStatusText("sin ACK / error", wxString::FromUTF8(detail.c_str()));
    ApplyStatusStyle(wxColour(219, 38, 9));
    connectButton_->SetLabel("Reconectar");
    connectButton_->Enable(true);
    portCtrl_->Enable(true);
}

void ConnectionPanel::SetConnectionLost() {
    SetStatusText("conexion perdida");
    ApplyStatusStyle(wxColour(219, 38, 9));
    connectButton_->SetLabel("Reconectar");
    connectButton_->Enable(true);
    portCtrl_->Enable(true);
}

std::string ConnectionPanel::GetPortName() const {
    if (!portCtrl_) {
        return std::string();
    }

    std::string portName = TrimAscii(portCtrl_->GetValue().ToStdString());
    if (!portName.empty()) {
        return portName;
    }

    portName = TrimAscii(portCtrl_->GetStringSelection().ToStdString());
    if (!portName.empty()) {
        return portName;
    }

    if (portCtrl_->GetCount() > 0) {
        portName = TrimAscii(portCtrl_->GetString(0).ToStdString());
        if (!portName.empty()) {
            return portName;
        }
    }

    return std::string();
}

wxButton* ConnectionPanel::GetConnectButton() const {
    return connectButton_;
}

} // namespace batview::ui::panels
