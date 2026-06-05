#pragma once

#ifdef BATVIEW_ENABLE_WX

#include <wx/dialog.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/button.h>

#include <string>

namespace batview::ui::dialogs {

class ExportDialog : public wxDialog {
public:
    explicit ExportDialog(wxWindow* parent);

    std::string GetFilePath() const;
    std::string GetExportFormat() const;

private:
    wxTextCtrl* filePathCtrl_;
    wxChoice* formatChoice_;
};

} // namespace batview::ui::dialogs

#endif
