#pragma once

#include <wx/animate.h>
#include <wx/gauge.h>
#include <wx/frame.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/radiobut.h>
#include <wx/simplebook.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/timer.h>

#include "ui/panels/ConnectionPanel.h"
#include "ui/panels/PlotPanel.h"
#include "ui/viewmodels/MainViewModel.h"
#include "ui/dialogs/ExportDialog.h"

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
    void OnSplashTimer(wxTimerEvent& event);
    void OnConnectButton(wxCommandEvent& event);
    void OnBatteryChanged(wxCommandEvent& event);
    void OnFunctionChanged(wxCommandEvent& event);
    void OnCycleModeChanged(wxCommandEvent& event);
    void OnBackStep(wxCommandEvent& event);
    void OnNextStep(wxCommandEvent& event);
    void OnStartOperation(wxCommandEvent& event);
    void OnStopOperation(wxCommandEvent& event);
    void OnExportData(wxCommandEvent& event);

    int GetSelectedBatteryTypeCode() const;
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

    wxPanel* flowPanel_;
    batview::ui::panels::ConnectionPanel* connectionPanel_;
    wxStaticText* batteryLabel_;
    wxChoice* batteryChoice_;
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
};

} // namespace batview::ui::frames
