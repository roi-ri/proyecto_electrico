#include "ui/frames/MainFrame.h"

#include <algorithm>
#include <thread>

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/msgdlg.h>
#include <wx/radiobut.h>
#include <wx/simplebook.h>

#include "ui/dialogs/ExportDialog.h"
#include "ui/panels/ConnectionPanel.h"
#include "ui/viewmodels/MainViewModel.h"

namespace batview::ui::frames {

void MainFrame::OnSplashTimer(wxTimerEvent& event) {
    (void)event;
    pageBook_->SetSelection(1);
}

void MainFrame::OnConnectButton(wxCommandEvent& event) {
    (void)event;
    if (!viewModel_) {
        AppendTraffic(true, "No hay ViewModel inicializado para conectar.");
        return;
    }

    if (connectionInProgress_) {
        AppendTraffic(true, "Ya hay una conexion en progreso.");
        return;
    }

    ClearTrafficLog();
    connectionPanel_->SetConnecting();

    connectionPanel_->RefreshPorts();

    const std::string portName = connectionPanel_->GetPortName();
    if (portName.empty()) {
        AppendTraffic(true, "No se selecciono un puerto serial valido.");
        connectionPanel_->SetConnectionLost();
        wxMessageBox("Ingrese un puerto serial valido.", "batView", wxICON_WARNING | wxOK, this);
        return;
    }

    AppendTraffic(true, "Intentando conectar al puerto " + portName);

    connectionInProgress_ = true;

    auto viewModel = viewModel_;
    std::thread([this, viewModel, portName]() {
        const bool connected = viewModel->ConnectToDevice(portName);
        const std::string errorDetail = viewModel->GetLastCommunicationError();

        CallAfter([this, connected, errorDetail]() {
            connectionInProgress_ = false;
            isConnected_ = connected;

            if (connected) {
                AppendTraffic(true, "Conexion completada correctamente.");
                connectionPanel_->SetConnected();
                wizardStep_ = 1;
            } else {
                AppendTraffic(true, errorDetail.empty() ? "La conexion fallo sin detalle adicional."
                                                        : "La conexion fallo: " + errorDetail);
                connectionPanel_->SetConnectionError(errorDetail);
                connectionPanel_->RefreshPorts();
                wizardStep_ = 0;
                const wxString message = errorDetail.empty()
                    ? "No se pudo conectar con el ESP32."
                    : wxString::FromUTF8(errorDetail.c_str());
                wxMessageBox(message, "batView", wxICON_ERROR | wxOK, this);
            }

            ResetWorkflowState();
            UpdateFlowVisibility();
            UpdateFunctionOptions();
        });
    }).detach();
}

void MainFrame::OnBatteryProfileChanged(wxCommandEvent& event) {
    (void)event;
    batteryChosen_ = false;
    functionChosen_ = false;
    operationActive_ = false;

    const int selection = batteryProfileChoice_ ? batteryProfileChoice_->GetSelection() : wxNOT_FOUND;
    if (selection != wxNOT_FOUND) {
        LoadBatteryProfileForm(static_cast<std::size_t>(selection));
    }

    UpdateFlowVisibility();
    UpdateFunctionOptions();
}

void MainFrame::OnSaveBatteryProfile(wxCommandEvent& event) {
    (void)event;
    batview::core::protocol::BatteryProfile profile;
    wxString error;
    if (!ReadBatteryProfileForm(profile, error)) {
        wxMessageBox(error, "batView", wxICON_WARNING | wxOK, this);
        return;
    }

    auto existing = std::find_if(batteryProfiles_.begin(), batteryProfiles_.end(),
                                 [&profile](const auto& candidate) {
                                     return candidate.nameId == profile.nameId;
                                 });
    if (existing == batteryProfiles_.end()) {
        batteryProfiles_.push_back(profile);
    } else {
        *existing = profile;
    }

    RefreshBatteryProfileChoices();
    const auto selected = std::find_if(batteryProfiles_.begin(), batteryProfiles_.end(),
                                       [&profile](const auto& candidate) {
                                           return candidate.nameId == profile.nameId;
                                       });
    if (selected != batteryProfiles_.end()) {
        batteryProfileChoice_->SetSelection(static_cast<int>(std::distance(batteryProfiles_.begin(), selected)));
    }

    batteryChosen_ = false;
    functionChosen_ = false;
    operationActive_ = false;
    AppendTraffic(true, "Perfil de bateria listo: " + profile.nameId);
    UpdateFlowVisibility();
    UpdateFunctionOptions();
}

void MainFrame::OnChooseBatteryProfile(wxCommandEvent& event) {
    (void)event;
    if (!viewModel_) {
        return;
    }
    if (!isConnected_) {
        wxMessageBox("Conecte el ESP32 antes de elegir una bateria.", "batView", wxICON_WARNING | wxOK, this);
        return;
    }

    const int selection = batteryProfileChoice_ ? batteryProfileChoice_->GetSelection() : wxNOT_FOUND;
    if (selection == wxNOT_FOUND || selection < 0 ||
        static_cast<std::size_t>(selection) >= batteryProfiles_.size()) {
        wxMessageBox("Agregue y seleccione un perfil de bateria.", "batView", wxICON_WARNING | wxOK, this);
        return;
    }

    batview::core::protocol::BatteryProfile profile;
    wxString error;
    if (!ReadBatteryProfileForm(profile, error)) {
        wxMessageBox(error, "batView", wxICON_WARNING | wxOK, this);
        return;
    }

    batteryProfiles_[static_cast<std::size_t>(selection)] = profile;
    RefreshBatteryProfileChoices();
    batteryProfileChoice_->SetSelection(selection);

    if (!viewModel_->SendBatteryProfile(profile)) {
        ShowCommunicationFailure("No se recibio confirmacion del ESP32 para el perfil de bateria.");
        return;
    }

    batteryChosen_ = true;
    if (wizardStep_ < 2) {
        wizardStep_ = 2;
    }
    AppendTraffic(true, "Bateria elegida: " + profile.nameId);
    UpdateFlowVisibility();
    UpdateFunctionOptions();
}

void MainFrame::OnClearBatteryProfiles(wxCommandEvent& event) {
    (void)event;

    batteryProfiles_.clear();
    if (batteryProfileChoice_) {
        batteryProfileChoice_->Clear();
    }
    if (batteryNameCtrl_) {
        batteryNameCtrl_->Clear();
    }
    if (batteryVoltageMinCtrl_) {
        batteryVoltageMinCtrl_->Clear();
    }
    if (batteryVoltageMaxCtrl_) {
        batteryVoltageMaxCtrl_->Clear();
    }
    if (batteryMaxCurrentCtrl_) {
        batteryMaxCurrentCtrl_->Clear();
    }

    wizardStep_ = isConnected_ ? 1 : 0;
    ResetWorkflowState();
    AppendTraffic(true, "Battery profiles cleared.");
    UpdateFlowVisibility();
    UpdateFunctionOptions();
}

void MainFrame::OnFunctionChanged(wxCommandEvent& event) {
    (void)event;
    functionChosen_ = chargeRadio_->GetValue() || dischargeRadio_->GetValue() || cycleRadio_->GetValue();
    operationActive_ = false;
    isInfiniteRunning_ = false;
    if (functionChosen_ && wizardStep_ < 3) {
        wizardStep_ = 3;
    }
    UpdateFlowVisibility();
    UpdateFunctionOptions();
}

void MainFrame::OnCycleModeChanged(wxCommandEvent& event) {
    (void)event;
    operationActive_ = false;
    isInfiniteRunning_ = false;
    UpdateFunctionOptions();
}

void MainFrame::OnBackStep(wxCommandEvent& event) {
    (void)event;
    if (wizardStep_ > 0) {
        --wizardStep_;
    }
    UpdateFlowVisibility();
    UpdateFunctionOptions();
}

void MainFrame::OnNextStep(wxCommandEvent& event) {
    (void)event;

    bool canAdvance = false;
    if (wizardStep_ == 0) {
        canAdvance = isConnected_;
    } else if (wizardStep_ == 1) {
        canAdvance = batteryChosen_;
    } else if (wizardStep_ == 2) {
        canAdvance = functionChosen_;
    }

    if (wizardStep_ < 3 && canAdvance) {
        ++wizardStep_;
    }

    UpdateFlowVisibility();
    UpdateFunctionOptions();
}

void MainFrame::OnStartOperation(wxCommandEvent& event) {
    (void)event;
    if (!viewModel_) {
        return;
    }

    if (!isConnected_) {
        wxMessageBox("Conecte el ESP32 antes de iniciar.", "batView", wxICON_WARNING | wxOK, this);
        return;
    }

    if (!batteryChosen_ || !functionChosen_) {
        wxMessageBox("Seleccione bateria y funcion.", "batView", wxICON_WARNING | wxOK, this);
        return;
    }

    const int functionCode = GetSelectedFunctionCode();

    switch (functionCode) {
    case 1:
        if (!viewModel_->SendLoadTarget(GetTargetPercent())) {
            ShowCommunicationFailure("No se recibio #ACK,LOAD del ESP32.");
            return;
        }
        viewModel_->StartAcquisition();
        operationActive_ = true;
        isInfiniteRunning_ = false;
        break;
    case 2:
        if (!viewModel_->SendUnloadTarget(GetTargetPercent())) {
            ShowCommunicationFailure("No se recibio #ACK,UNLOAD del ESP32.");
            return;
        }
        viewModel_->StartAcquisition();
        operationActive_ = true;
        isInfiniteRunning_ = false;
        break;
    default:
        if (!viewModel_->SendCycleConfiguration(GetCycleInfiniteMode(), GetCycleCount())) {
            ShowCommunicationFailure("No se recibio #ACK,CICLE del ESP32.");
            return;
        }
        viewModel_->StartAcquisition();
        operationActive_ = true;
        isInfiniteRunning_ = GetCycleInfiniteMode();
        break;
    }

    UpdateFunctionOptions();
}

void MainFrame::OnStopOperation(wxCommandEvent& event) {
    (void)event;
    if (!viewModel_) {
        return;
    }

    if (!viewModel_->SendStopCommand()) {
        MarkConnectionLost();
        return;
    }

    viewModel_->StopAcquisition();
    operationActive_ = false;
    isInfiniteRunning_ = false;
    AppendTraffic(true, "Operacion detenida por el usuario.");
    UpdateFunctionOptions();
}

void MainFrame::OnExportData(wxCommandEvent& event) {
    (void)event;
    if (!viewModel_) {
        return;
    }

    batview::ui::dialogs::ExportDialog dialog(this);
    if (dialog.ShowModal() == wxID_OK) {
        std::string error;
        if (!viewModel_->ExportData(dialog.GetFilePath(), dialog.GetExportFormat(), error)) {
            wxMessageBox(wxString::FromUTF8(error.c_str()), "batView", wxICON_ERROR | wxOK, this);
            return;
        }

        wxMessageBox("Datos exportados correctamente.", "batView", wxICON_INFORMATION | wxOK, this);
    }
}

} // namespace batview::ui::frames
