#include "ui/dialogs/ExportDialog.h"

#ifdef BATVIEW_ENABLE_WX

#include <wx/stattext.h>
#include <wx/sizer.h>

namespace batview::ui::dialogs {

ExportDialog::ExportDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Exportar datos", wxDefaultPosition, wxSize(420, 180)) {
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    filePathCtrl_ = new wxTextCtrl(this, wxID_ANY, "data/session.csv");
    formatChoice_ = new wxChoice(this, wxID_ANY);
    formatChoice_->Append("CSV");
    formatChoice_->Append("MAT");
    formatChoice_->Append("XLSX");
    formatChoice_->SetSelection(0);

    auto* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->Add(new wxButton(this, wxID_OK, "Exportar"), 0, wxALL, 5);
    buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancelar"), 0, wxALL, 5);

    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Ruta de archivo"), 0, wxLEFT | wxTOP, 10);
    mainSizer->Add(filePathCtrl_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, "Formato"), 0, wxLEFT, 10);
    mainSizer->Add(formatChoice_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    mainSizer->Add(buttonSizer, 0, wxALIGN_RIGHT | wxRIGHT | wxBOTTOM, 10);

    SetSizerAndFit(mainSizer);
}

std::string ExportDialog::GetFilePath() const {
    return filePathCtrl_->GetValue().ToStdString();
}

std::string ExportDialog::GetExportFormat() const {
    return formatChoice_->GetStringSelection().ToStdString();
}

} // namespace batview::ui::dialogs

#endif
