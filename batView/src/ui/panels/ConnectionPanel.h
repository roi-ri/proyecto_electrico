#pragma once

#include <wx/panel.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace batview::ui::panels {

class ConnectionPanel : public wxPanel {
public:
    explicit ConnectionPanel(wxWindow* parent);

    void RefreshPorts();
    void SetDisconnected();
    void SetConnecting();
    void SetConnected();
    void SetConnectionError(const std::string& detail);
    void SetConnectionLost();
    std::string GetPortName() const;
    wxButton* GetConnectButton() const;

private:
    void ApplyStatusStyle(const wxColour& color);
    void SetStatusText(const wxString& text, const wxString& tooltip = wxString());

    wxStaticText* statusText_;
    wxComboBox* portCtrl_;
    wxButton* connectButton_;
};

} // namespace batview::ui::panels
