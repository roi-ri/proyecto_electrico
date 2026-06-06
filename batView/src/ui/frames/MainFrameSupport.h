#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <wx/string.h>

#include "core/protocol/ProtocolTypes.h"

class wxFrame;
class wxStaticText;
class wxTextCtrl;
class wxWindow;

namespace batview::ui::frames::detail {

void StyleTerminalTitle(wxStaticText* title);
void StyleTerminalTextBox(wxTextCtrl* control);
void PrepareFlatPanel(wxWindow* window);
void SetAppIcon(wxFrame& frame);
wxString FormatProfileLabel(const batview::core::protocol::BatteryProfile& profile);
std::string TrimmedUtf8(const wxString& value);
std::vector<std::filesystem::path> GetAssetDirectories();

} // namespace batview::ui::frames::detail
