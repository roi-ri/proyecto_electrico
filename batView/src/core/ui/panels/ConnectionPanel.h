#pragma once

#include <wx/panel.h>
#include <wx/button.h>
#include <wx/stattext.h>

namespace batview::ui::panels {

class ConnectionPanel : public wxPanel {
public:
    explicit ConnectionPanel(wxWindow* parent);

    void SetConnectionStatus(bool connected);
    void BindConnectionButton(wxCommandEventHandler handler);

private:
    wxStaticText* statusText_;
    wxButton* connectButton_;
};

} // namespace batview::ui::panels
