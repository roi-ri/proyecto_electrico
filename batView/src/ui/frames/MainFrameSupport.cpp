#include "ui/frames/MainFrameSupport.h"

#include <algorithm>
#include <sstream>

#include <wx/font.h>
#include <wx/frame.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/stdpaths.h>
#include <wx/textctrl.h>
#include <wx/window.h>

namespace batview::ui::frames::detail {

namespace {

const wxColour kTerminalBackground(10, 14, 18);
const wxColour kTerminalForeground(116, 255, 153);

bool TrySetFrameIcon(wxFrame& frame, const std::filesystem::path& iconPath, wxBitmapType iconType) {
    if (!std::filesystem::exists(iconPath)) {
        return false;
    }

    wxIcon icon(iconPath.string(), iconType);
    if (!icon.IsOk()) {
        return false;
    }

    frame.SetIcon(icon);
    return true;
}

} // namespace

void StyleTerminalTitle(wxStaticText* title) {
    if (!title) {
        return;
    }

    wxFont titleFont = title->GetFont();
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleFont.SetPointSize(titleFont.GetPointSize() + 1);
    title->SetFont(titleFont);
    title->SetForegroundColour(wxColour(220, 232, 240));
}

void StyleTerminalTextBox(wxTextCtrl* control) {
    if (!control) {
        return;
    }

    control->SetBackgroundColour(kTerminalBackground);
    control->SetForegroundColour(kTerminalForeground);
    control->SetFont(wxFontInfo(11).Family(wxFONTFAMILY_TELETYPE));
}

void PrepareFlatPanel(wxWindow* window) {
    if (!window) {
        return;
    }

    window->SetBackgroundStyle(wxBG_STYLE_SYSTEM);
}

void SetAppIcon(wxFrame& frame) {
    const auto assetDirectories = GetAssetDirectories();

#ifdef _WIN32
    for (const auto& assetDir : assetDirectories) {
        if (TrySetFrameIcon(frame, assetDir / "BatView.ico", wxBITMAP_TYPE_ICO)) {
            return;
        }
    }
#endif

    for (const auto& assetDir : assetDirectories) {
        if (TrySetFrameIcon(frame, assetDir / "BatView.png", wxBITMAP_TYPE_PNG)) {
            return;
        }
    }
}

wxString FormatProfileLabel(const batview::core::protocol::BatteryProfile& profile) {
    std::ostringstream label;
    label << profile.nameId << "  |  Tension max (V) " << profile.voltageAtMax
          << "  |  Tension min (V) " << profile.voltageAtMin
          << "  |  Corriente max (A) " << profile.maxCurrent;
    return wxString::FromUTF8(label.str().c_str());
}

std::string TrimmedUtf8(const wxString& value) {
    wxString trimmed = value;
    trimmed.Trim(true);
    trimmed.Trim(false);
    return trimmed.ToStdString();
}

std::vector<std::filesystem::path> GetAssetDirectories() {
    std::vector<std::filesystem::path> directories;

    const wxStandardPaths& standardPaths = wxStandardPaths::Get();
    directories.emplace_back(standardPaths.GetResourcesDir().ToStdString());

    const std::filesystem::path executablePath(standardPaths.GetExecutablePath().ToStdString());
    if (!executablePath.empty()) {
        const std::filesystem::path executableDir = executablePath.parent_path();
        directories.push_back(executableDir);
        directories.push_back(executableDir / "assets");
    }

    std::error_code error;
    const auto cwd = std::filesystem::current_path(error);
    if (!error) {
        directories.push_back(cwd);
        directories.push_back(cwd / "assets");
    }

    std::sort(directories.begin(), directories.end());
    directories.erase(std::unique(directories.begin(), directories.end()), directories.end());
    return directories;
}

} // namespace batview::ui::frames::detail
