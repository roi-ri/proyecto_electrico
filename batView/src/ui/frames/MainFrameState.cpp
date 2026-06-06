#include "ui/frames/MainFrame.h"

#include <algorithm>

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/gauge.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/simplebook.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>

#include "ui/frames/MainFrameSupport.h"
#include "ui/panels/ConnectionPanel.h"
#include "ui/viewmodels/MainViewModel.h"

namespace batview::ui::frames {

void MainFrame::UpdateFlowVisibility() {
    const bool showBattery = isConnected_ && wizardStep_ >= 1;
    const bool showFunction = isConnected_ && batteryChosen_ && wizardStep_ >= 2;
    const bool showOptions = isConnected_ && batteryChosen_ && functionChosen_ && wizardStep_ >= 3;

    batteryPanel_->Show(showBattery);
    functionPanel_->Show(showFunction);
    optionsPanel_->Show(showOptions);
    UpdateWizardNavigation();
    flowPanel_->Layout();
    if (workflowSplitter_) {
        workflowSplitter_->Layout();
    }
    workflowPage_->Layout();
    pageBook_->Layout();
    Layout();
    mainPanel_->Layout();
}

void MainFrame::ClearTrafficLog() {
    if (!trafficLogCtrl_) {
        return;
    }

    trafficLogCtrl_->Clear();
}

void MainFrame::ResetWorkflowState() {
    batteryChosen_ = false;
    functionChosen_ = false;
    operationActive_ = false;
    isInfiniteRunning_ = false;
    chargeRadio_->SetValue(false);
    dischargeRadio_->SetValue(false);
    cycleRadio_->SetValue(false);
    if (batteryProfileChoice_) {
        batteryProfileChoice_->SetSelection(wxNOT_FOUND);
    }
}

void MainFrame::UpdateFunctionOptions() {
    const int functionCode = GetSelectedFunctionCode();
    const bool cycleMode = functionCode == 3;
    const bool infiniteMode = GetCycleInfiniteMode();

    targetPercentLabel_->Show(!cycleMode);
    targetPercentCtrl_->Show(!cycleMode);

    cycleModeLabel_->Show(cycleMode);
    cycleModeChoice_->Show(cycleMode);

    const bool showCycleCount = cycleMode && !infiniteMode;
    cycleCountLabel_->Show(showCycleCount);
    cycleCountCtrl_->Show(showCycleCount);

    stopButton_->Show(operationActive_);
    startButton_->Enable(isConnected_ && batteryChosen_ && functionChosen_ && !operationActive_);
    optionsPanel_->Layout();
    flowPanel_->Layout();
}

void MainFrame::MarkConnectionLost() {
    isConnected_ = false;
    connectionPanel_->SetConnectionLost();
    wizardStep_ = 0;
    ResetWorkflowState();
    UpdateFlowVisibility();
    UpdateFunctionOptions();
    wxMessageBox("conexion perdida", "batView", wxICON_WARNING | wxOK, this);
}

void MainFrame::ShowCommunicationFailure(const std::string& fallbackMessage) {
    const std::string detail = viewModel_ ? viewModel_->GetLastCommunicationError() : std::string();
    const std::string message = detail.empty() ? fallbackMessage : detail;

    isConnected_ = false;
    connectionPanel_->SetConnectionError(message);
    wizardStep_ = 0;
    ResetWorkflowState();
    UpdateFlowVisibility();
    UpdateFunctionOptions();
    wxMessageBox(wxString::FromUTF8(message.c_str()), "batView", wxICON_ERROR | wxOK, this);
}

void MainFrame::UpdateWizardNavigation() {
    if (!wizardProgress_ || !backStepButton_ || !nextStepButton_) {
        return;
    }

    wizardProgress_->SetRange(4);
    wizardProgress_->SetValue(std::clamp(wizardStep_ + 1, 1, 4));

    backStepButton_->Enable(wizardStep_ > 0);

    bool canAdvance = false;
    if (wizardStep_ == 0) {
        canAdvance = isConnected_;
    } else if (wizardStep_ == 1) {
        canAdvance = batteryChosen_;
    } else if (wizardStep_ == 2) {
        canAdvance = functionChosen_;
    }

    nextStepButton_->Enable(wizardStep_ < 3 && canAdvance);
}

void MainFrame::RefreshBatteryProfileChoices() {
    if (!batteryProfileChoice_) {
        return;
    }

    batteryProfileChoice_->Clear();
    for (const auto& profile : batteryProfiles_) {
        batteryProfileChoice_->Append(detail::FormatProfileLabel(profile));
    }
}

bool MainFrame::ReadBatteryProfileForm(batview::core::protocol::BatteryProfile& outProfile,
                                       wxString& outError) const {
    if (!batteryNameCtrl_ || !batteryVoltageMaxCtrl_ || !batteryVoltageMinCtrl_ || !batteryMaxCurrentCtrl_) {
        outError = "El formulario de bateria no esta listo.";
        return false;
    }

    outProfile.nameId = detail::TrimmedUtf8(batteryNameCtrl_->GetValue());
    if (outProfile.nameId.empty()) {
        outError = "Ingrese un BatteryNameID.";
        return false;
    }
    if (outProfile.nameId.find(',') != std::string::npos) {
        outError = "BatteryNameID no puede contener comas.";
        return false;
    }

    if (!batteryVoltageMaxCtrl_->GetValue().ToDouble(&outProfile.voltageAtMax)) {
        outError = "Ingrese un V@max numerico.";
        return false;
    }
    if (!batteryVoltageMinCtrl_->GetValue().ToDouble(&outProfile.voltageAtMin)) {
        outError = "Ingrese un V@min numerico.";
        return false;
    }
    if (!batteryMaxCurrentCtrl_->GetValue().ToDouble(&outProfile.maxCurrent)) {
        outError = "Ingrese un Amax numerico.";
        return false;
    }
    if (outProfile.voltageAtMax <= 0.0 || outProfile.voltageAtMin <= 0.0 || outProfile.maxCurrent <= 0.0) {
        outError = "V@max, V@min y Amax deben ser mayores que cero.";
        return false;
    }
    if (outProfile.voltageAtMin >= outProfile.voltageAtMax) {
        outError = "V@min debe ser menor que V@max.";
        return false;
    }

    return true;
}

void MainFrame::LoadBatteryProfileForm(std::size_t index) {
    if (index >= batteryProfiles_.size()) {
        return;
    }

    const auto& profile = batteryProfiles_[index];
    batteryNameCtrl_->SetValue(wxString::FromUTF8(profile.nameId.c_str()));
    batteryVoltageMaxCtrl_->SetValue(wxString::Format("%.3f", profile.voltageAtMax));
    batteryVoltageMinCtrl_->SetValue(wxString::Format("%.3f", profile.voltageAtMin));
    batteryMaxCurrentCtrl_->SetValue(wxString::Format("%.3f", profile.maxCurrent));
}

void MainFrame::AppendTraffic(bool outgoing, const std::string& message) {
    if (!trafficLogCtrl_) {
        return;
    }

    const wxString prefix = outgoing ? "PC: " : "ESP32: ";
    const wxString line = prefix + wxString::FromUTF8(message.c_str()) + "\n";

    CallAfter([this, line]() {
        if (!trafficLogCtrl_) {
            return;
        }

        trafficLogCtrl_->AppendText(line);
        trafficLogCtrl_->ShowPosition(trafficLogCtrl_->GetLastPosition());
    });
}

int MainFrame::GetSelectedFunctionCode() const {
    if (chargeRadio_ && chargeRadio_->GetValue()) {
        return 1;
    }
    if (dischargeRadio_ && dischargeRadio_->GetValue()) {
        return 2;
    }
    if (cycleRadio_ && cycleRadio_->GetValue()) {
        return 3;
    }
    return 0;
}

bool MainFrame::GetCycleInfiniteMode() const {
    return cycleModeChoice_ && cycleModeChoice_->GetSelection() == 1;
}

int MainFrame::GetCycleCount() const {
    long value = 1;
    if (cycleCountCtrl_) {
        cycleCountCtrl_->GetValue().ToLong(&value);
    }
    return static_cast<int>(value);
}

int MainFrame::GetTargetPercent() const {
    long value = 80;
    if (targetPercentCtrl_) {
        targetPercentCtrl_->GetValue().ToLong(&value);
    }
    return static_cast<int>(value);
}

} // namespace batview::ui::frames
