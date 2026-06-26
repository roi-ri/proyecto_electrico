#pragma once

#include <wx/frame.h>
#include <wx/string.h>
#include <wx/timer.h>

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "core/protocol/ProtocolTypes.h"

class wxAnimationCtrl;
class wxButton;
class wxChoice;
class wxCommandEvent;
class wxGauge;
class wxPanel;
class wxRadioButton;
class wxScrolledWindow;
class wxSimplebook;
class wxSplitterWindow;
class wxStaticText;
class wxTextCtrl;
class wxTimerEvent;
class wxWindow;

namespace batview::ui::panels {
class ConnectionPanel;
class PlotPanel;
} // namespace batview::ui::panels

namespace batview::ui::viewmodels {
class MainViewModel;
} // namespace batview::ui::viewmodels

namespace batview::ui::frames {

class MainFrame : public wxFrame {
public:
    explicit MainFrame(const wxString& title);

    void SetViewModel(std::shared_ptr<batview::ui::viewmodels::MainViewModel> viewModel);

private:
    void BuildLayout();
    void BuildSplashPage(wxWindow* parent);
    void BuildWorkflowPage(wxWindow* parent);
    void BindEvents();
    void ConfigureBrandStyle();
    void UpdateFlowVisibility();
    void UpdateFunctionOptions();
    void UpdateWizardNavigation();
    void ClearTrafficLog();
    void ResetWorkflowState();
    void MarkConnectionLost();
    void ShowCommunicationFailure(const std::string& fallbackMessage);
    void AppendTraffic(bool outgoing, const std::string& message);
    void RefreshBatteryProfileChoices();
    void LoadStoredBatteryProfiles();
    void SaveStoredBatteryProfiles() const;
    bool ReadBatteryProfileForm(batview::core::protocol::BatteryProfile& outProfile,
                                wxString& outError) const;
    void LoadBatteryProfileForm(std::size_t index);
    void OnSplashTimer(wxTimerEvent& event);
    void OnConnectButton(wxCommandEvent& event);
    void OnDisconnectButton(wxCommandEvent& event);
    void OnBatteryProfileChanged(wxCommandEvent& event);
    void OnSaveBatteryProfile(wxCommandEvent& event);
    void OnChooseBatteryProfile(wxCommandEvent& event);
    void OnClearBatteryProfiles(wxCommandEvent& event);
    void OnFunctionChanged(wxCommandEvent& event);
    void OnCycleModeChanged(wxCommandEvent& event);
    void OnBackStep(wxCommandEvent& event);
    void OnNextStep(wxCommandEvent& event);
    void OnStartOperation(wxCommandEvent& event);
    void OnStopOperation(wxCommandEvent& event);
    void OnExportData(wxCommandEvent& event);

    int GetSelectedFunctionCode() const;
    bool GetCycleInfiniteMode() const;
    int GetCycleCount() const;
    int GetTargetPercent() const;

    wxPanel* mainPanel_;
    wxSplitterWindow* workspaceSplitter_;
    wxSplitterWindow* workflowSplitter_;
    wxSimplebook* pageBook_;
    wxPanel* splashPage_;
    wxPanel* workflowPage_;

    wxAnimationCtrl* splashAnimation_;
    wxTimer splashTimer_;

    wxScrolledWindow* flowPanel_;
    batview::ui::panels::ConnectionPanel* connectionPanel_;
    wxPanel* batteryPanel_;
    wxChoice* batteryProfileChoice_;
    wxTextCtrl* batteryNameCtrl_;
    wxTextCtrl* batteryVoltageMaxCtrl_;
    wxTextCtrl* batteryVoltageMinCtrl_;
    wxTextCtrl* batteryMaxCurrentCtrl_;
    wxButton* saveBatteryProfileButton_;
    wxButton* chooseBatteryProfileButton_;
    wxButton* clearBatteryProfilesButton_;
    wxPanel* functionPanel_;
    wxRadioButton* chargeRadio_;
    wxRadioButton* dischargeRadio_;
    wxRadioButton* cycleRadio_;

    wxPanel* optionsPanel_;
    wxStaticText* cycleModeLabel_;
    wxChoice* cycleModeChoice_;
    wxStaticText* cycleCountLabel_;
    wxTextCtrl* cycleCountCtrl_;
    wxStaticText* targetPercentLabel_;
    wxTextCtrl* targetPercentCtrl_;
    wxButton* startButton_;
    wxButton* stopButton_;
    wxButton* exportButton_;
    batview::ui::panels::PlotPanel* plotPanel_;

    wxTextCtrl* trafficLogCtrl_;

    wxGauge* wizardProgress_;
    wxButton* backStepButton_;
    wxButton* nextStepButton_;

    std::shared_ptr<batview::ui::viewmodels::MainViewModel> viewModel_;
    int wizardStep_;
    bool isConnected_;
    bool batteryChosen_;
    bool functionChosen_;
    bool operationActive_;
    bool isInfiniteRunning_;
    bool connectionInProgress_;
    std::vector<batview::core::protocol::BatteryProfile> batteryProfiles_;
};

} // namespace batview::ui::frames
