#include "ui/frames/MainFrame.h"

#include <filesystem>

#include <wx/animate.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/font.h>
#include <wx/gauge.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/simplebook.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>
#include <wx/window.h>

#include "ui/frames/MainFrameSupport.h"
#include "ui/panels/ConnectionPanel.h"
#include "ui/panels/PlotPanel.h"

namespace batview::ui::frames {

namespace {

const int kRadioButtonMinHeight = 32;
const int kFunctionPanelMinHeight = 88;

} // namespace

void MainFrame::BuildLayout() {
    mainPanel_ = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    detail::PrepareFlatPanel(mainPanel_);

    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    workspaceSplitter_ = new wxSplitterWindow(mainPanel_, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                              wxSP_LIVE_UPDATE | wxSP_NO_XP_THEME);
    workspaceSplitter_->SetMinimumPaneSize(260);

    auto* leftPane = new wxPanel(workspaceSplitter_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    detail::PrepareFlatPanel(leftPane);

    auto* rightPane = new wxPanel(workspaceSplitter_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    detail::PrepareFlatPanel(rightPane);

    pageBook_ = new wxSimplebook(leftPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    BuildSplashPage(pageBook_);
    BuildWorkflowPage(pageBook_);

    auto* leftSizer = new wxBoxSizer(wxVERTICAL);
    leftSizer->Add(pageBook_, 1, wxEXPAND);
    leftPane->SetSizer(leftSizer);

    auto* rightSizer = new wxBoxSizer(wxVERTICAL);
    plotPanel_ = new batview::ui::panels::PlotPanel(rightPane);
    rightSizer->Add(plotPanel_, 1, wxEXPAND);
    rightPane->SetSizer(rightSizer);

    workspaceSplitter_->SplitVertically(leftPane, rightPane, 830);
    workspaceSplitter_->SetSashGravity(0.68);

    mainSizer->Add(workspaceSplitter_, 1, wxEXPAND);
    mainPanel_->SetSizer(mainSizer);

    pageBook_->SetSelection(0);
    splashTimer_.StartOnce(2600);
    UpdateFlowVisibility();
}

void MainFrame::BuildSplashPage(wxWindow* parent) {
    splashPage_ = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    splashPage_->SetBackgroundStyle(wxBG_STYLE_SYSTEM);

    auto* title = new wxStaticText(splashPage_, wxID_ANY, "batView");
    wxFont titleFont = title->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 16);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);

    auto* subtitle = new wxStaticText(splashPage_, wxID_ANY, "Energia inteligente para pruebas de baterias");

    splashAnimation_ = new wxAnimationCtrl(splashPage_, wxID_ANY);
    splashAnimation_->SetInactiveBitmap(wxBitmap(380, 230));

    for (const auto& assetDir : detail::GetAssetDirectories()) {
        const std::filesystem::path gifPath = assetDir / "batView.gif";
        if (std::filesystem::exists(gifPath) && splashAnimation_->LoadFile(gifPath.string())) {
            splashAnimation_->Play();
            break;
        }
    }

    auto* launchLabel = new wxStaticText(splashPage_, wxID_ANY, "Inicializando interfaz...");

    auto* splashSizer = new wxBoxSizer(wxVERTICAL);
    splashSizer->AddStretchSpacer(1);
    splashSizer->Add(title, 0, wxALIGN_CENTER | wxBOTTOM, 8);
    splashSizer->Add(subtitle, 0, wxALIGN_CENTER | wxBOTTOM, 16);
    splashSizer->Add(splashAnimation_, 0, wxALIGN_CENTER | wxBOTTOM, 14);
    splashSizer->Add(launchLabel, 0, wxALIGN_CENTER | wxBOTTOM, 10);
    splashSizer->AddStretchSpacer(1);
    splashPage_->SetSizer(splashSizer);

    pageBook_->AddPage(splashPage_, "Splash", true);
}

void MainFrame::BuildWorkflowPage(wxWindow* parent) {
    workflowPage_ = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    detail::PrepareFlatPanel(workflowPage_);
    workflowSplitter_ = new wxSplitterWindow(workflowPage_, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                             wxSP_LIVE_UPDATE | wxSP_NO_XP_THEME);
    workflowSplitter_->SetMinimumPaneSize(160);
    workflowSplitter_->SetSashGravity(0.62);

    flowPanel_ = new wxPanel(workflowSplitter_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    detail::PrepareFlatPanel(flowPanel_);

    connectionPanel_ = new batview::ui::panels::ConnectionPanel(flowPanel_);

    batteryPanel_ = new wxPanel(flowPanel_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    detail::PrepareFlatPanel(batteryPanel_);
    auto* batteryTitle = new wxStaticText(batteryPanel_, wxID_ANY, "Battery profiles");
    wxFont batteryTitleFont = batteryTitle->GetFont();
    batteryTitleFont.SetWeight(wxFONTWEIGHT_BOLD);
    batteryTitle->SetFont(batteryTitleFont);

    batteryProfileChoice_ = new wxChoice(batteryPanel_, wxID_ANY);
    batteryProfileChoice_->SetSelection(wxNOT_FOUND);

    batteryNameCtrl_ = new wxTextCtrl(batteryPanel_, wxID_ANY);
    batteryNameCtrl_->SetHint("Profile name");
    batteryVoltageMaxCtrl_ = new wxTextCtrl(batteryPanel_, wxID_ANY);
    batteryVoltageMaxCtrl_->SetHint("V at max charge");
    batteryVoltageMinCtrl_ = new wxTextCtrl(batteryPanel_, wxID_ANY);
    batteryVoltageMinCtrl_->SetHint("V at min charge");
    batteryMaxCurrentCtrl_ = new wxTextCtrl(batteryPanel_, wxID_ANY);
    batteryMaxCurrentCtrl_->SetHint("Max current");
    saveBatteryProfileButton_ = new wxButton(batteryPanel_, wxID_ANY, "Add / update profile");
    chooseBatteryProfileButton_ = new wxButton(batteryPanel_, wxID_ANY, "Choose battery");
    clearBatteryProfilesButton_ = new wxButton(batteryPanel_, wxID_ANY, "Clear profiles");

    auto* batteryGrid = new wxFlexGridSizer(2, 8, 8);
    batteryGrid->AddGrowableCol(1, 1);
    batteryGrid->Add(new wxStaticText(batteryPanel_, wxID_ANY, "Profile name"), 0, wxALIGN_CENTER_VERTICAL);
    batteryGrid->Add(batteryNameCtrl_, 1, wxEXPAND);
    batteryGrid->Add(new wxStaticText(batteryPanel_, wxID_ANY, "V at min charge"), 0, wxALIGN_CENTER_VERTICAL);
    batteryGrid->Add(batteryVoltageMinCtrl_, 1, wxEXPAND);
    batteryGrid->Add(new wxStaticText(batteryPanel_, wxID_ANY, "V at max charge"), 0, wxALIGN_CENTER_VERTICAL);
    batteryGrid->Add(batteryVoltageMaxCtrl_, 1, wxEXPAND);
    batteryGrid->Add(new wxStaticText(batteryPanel_, wxID_ANY, "Max current"), 0, wxALIGN_CENTER_VERTICAL);
    batteryGrid->Add(batteryMaxCurrentCtrl_, 1, wxEXPAND);

    auto* batteryActions = new wxBoxSizer(wxHORIZONTAL);
    batteryActions->Add(saveBatteryProfileButton_, 0, wxRIGHT, 8);
    batteryActions->Add(chooseBatteryProfileButton_, 0, wxRIGHT, 8);
    batteryActions->Add(clearBatteryProfilesButton_, 0);

    auto* batterySizer = new wxBoxSizer(wxVERTICAL);
    batterySizer->Add(batteryTitle, 0, wxBOTTOM, 6);
    batterySizer->Add(batteryGrid, 0, wxEXPAND | wxBOTTOM, 8);
    batterySizer->Add(new wxStaticText(batteryPanel_, wxID_ANY, "Saved profile"), 0, wxBOTTOM, 4);
    batterySizer->Add(batteryProfileChoice_, 0, wxEXPAND | wxBOTTOM, 8);
    batterySizer->Add(batteryActions, 0);
    batteryPanel_->SetSizer(batterySizer);

    functionPanel_ = new wxPanel(flowPanel_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    detail::PrepareFlatPanel(functionPanel_);
    auto* functionLabel = new wxStaticText(functionPanel_, wxID_ANY, "Funcion");
    chargeRadio_ = new wxRadioButton(functionPanel_, wxID_ANY, "Carga", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    dischargeRadio_ = new wxRadioButton(functionPanel_, wxID_ANY, "Descarga");
    cycleRadio_ = new wxRadioButton(functionPanel_, wxID_ANY, "Ciclado");
    chargeRadio_->SetMinSize(wxSize(-1, kRadioButtonMinHeight));
    dischargeRadio_->SetMinSize(wxSize(-1, kRadioButtonMinHeight));
    cycleRadio_->SetMinSize(wxSize(-1, kRadioButtonMinHeight));
    chargeRadio_->SetValue(false);
    dischargeRadio_->SetValue(false);
    cycleRadio_->SetValue(false);

    auto* functionRow = new wxBoxSizer(wxHORIZONTAL);
    functionRow->Add(chargeRadio_, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 14);
    functionRow->Add(dischargeRadio_, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 14);
    functionRow->Add(cycleRadio_, 0, wxALIGN_CENTER_VERTICAL);

    auto* functionSizer = new wxBoxSizer(wxVERTICAL);
    functionSizer->Add(functionLabel, 0, wxBOTTOM, 4);
    functionSizer->Add(functionRow, 0, wxEXPAND | wxTOP, 2);
    functionPanel_->SetSizer(functionSizer);
    functionPanel_->SetMinSize(wxSize(-1, kFunctionPanelMinHeight));

    optionsPanel_ = new wxPanel(flowPanel_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    detail::PrepareFlatPanel(optionsPanel_);
    cycleModeLabel_ = new wxStaticText(optionsPanel_, wxID_ANY, "Modo de ciclos");
    cycleModeChoice_ = new wxChoice(optionsPanel_, wxID_ANY);
    cycleModeChoice_->Append("Finitos");
    cycleModeChoice_->Append("Infinitos");
    cycleModeChoice_->SetSelection(0);

    cycleCountLabel_ = new wxStaticText(optionsPanel_, wxID_ANY, "Numero de ciclos");
    cycleCountCtrl_ = new wxTextCtrl(optionsPanel_, wxID_ANY, "3");
    targetPercentLabel_ = new wxStaticText(optionsPanel_, wxID_ANY, "Porcentaje objetivo");
    targetPercentCtrl_ = new wxTextCtrl(optionsPanel_, wxID_ANY, "80");

    startButton_ = new wxButton(optionsPanel_, wxID_ANY, "Iniciar");
    stopButton_ = new wxButton(optionsPanel_, wxID_ANY, "Detener");
    exportButton_ = new wxButton(optionsPanel_, wxID_ANY, "Exportar Datos");

    auto* optionsSizer = new wxBoxSizer(wxVERTICAL);
    optionsSizer->Add(cycleModeLabel_, 0, wxTOP | wxBOTTOM, 4);
    optionsSizer->Add(cycleModeChoice_, 0, wxEXPAND | wxBOTTOM, 8);
    optionsSizer->Add(cycleCountLabel_, 0, wxTOP | wxBOTTOM, 4);
    optionsSizer->Add(cycleCountCtrl_, 0, wxEXPAND | wxBOTTOM, 8);
    optionsSizer->Add(targetPercentLabel_, 0, wxTOP | wxBOTTOM, 4);
    optionsSizer->Add(targetPercentCtrl_, 0, wxEXPAND | wxBOTTOM, 10);

    auto* actionsSizer = new wxBoxSizer(wxHORIZONTAL);
    actionsSizer->Add(startButton_, 0, wxRIGHT, 8);
    actionsSizer->Add(stopButton_, 0, wxRIGHT, 8);
    actionsSizer->Add(exportButton_, 0);
    optionsSizer->Add(actionsSizer, 0, wxTOP, 4);
    optionsPanel_->SetSizer(optionsSizer);

    wizardProgress_ = new wxGauge(workflowPage_, wxID_ANY, 4, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL | wxGA_SMOOTH);
    wizardProgress_->SetMinSize(wxSize(-1, 18));

    backStepButton_ = new wxButton(workflowPage_, wxID_ANY, "Atras");
    nextStepButton_ = new wxButton(workflowPage_, wxID_ANY, "Siguiente");

    auto* wizardButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    wizardButtonsSizer->Add(backStepButton_, 0, wxRIGHT, 8);
    wizardButtonsSizer->Add(nextStepButton_, 0);

    auto* flowSizer = new wxBoxSizer(wxVERTICAL);
    flowSizer->Add(connectionPanel_, 0, wxEXPAND | wxBOTTOM, 8);
    flowSizer->Add(new wxStaticLine(flowPanel_, wxID_ANY), 0, wxEXPAND | wxBOTTOM, 8);
    flowSizer->Add(batteryPanel_, 0, wxEXPAND | wxBOTTOM, 10);
    flowSizer->Add(functionPanel_, 0, wxEXPAND | wxBOTTOM, 10);
    flowSizer->Add(optionsPanel_, 0, wxEXPAND | wxBOTTOM, 10);
    flowPanel_->SetSizer(flowSizer);

    auto* trafficPane = new wxPanel(workflowSplitter_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    detail::PrepareFlatPanel(trafficPane);
    auto* trafficSizer = new wxBoxSizer(wxVERTICAL);
    auto* trafficTitle = new wxStaticText(trafficPane, wxID_ANY, "Mensajes ESP32 / PC");
    detail::StyleTerminalTitle(trafficTitle);
    trafficLogCtrl_ = new wxTextCtrl(trafficPane, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 220),
                                     wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 | wxTE_DONTWRAP | wxHSCROLL |
                                         wxBORDER_NONE);
    detail::StyleTerminalTextBox(trafficLogCtrl_);
    trafficSizer->Add(trafficTitle, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    trafficSizer->Add(trafficLogCtrl_, 1, wxEXPAND | wxALL, 10);
    trafficPane->SetSizer(trafficSizer);

    workflowSplitter_->SplitHorizontally(flowPanel_, trafficPane, 520);

    auto* workflowSizer = new wxBoxSizer(wxVERTICAL);
    workflowSizer->Add(wizardProgress_, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 8);
    workflowSizer->Add(wizardButtonsSizer, 0, wxLEFT | wxTOP | wxBOTTOM, 8);
    workflowSizer->Add(workflowSplitter_, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    workflowPage_->SetSizer(workflowSizer);

    pageBook_->AddPage(workflowPage_, "Workflow", false);
}

void MainFrame::BindEvents() {
    Bind(wxEVT_TIMER, &MainFrame::OnSplashTimer, this);
    connectionPanel_->GetConnectButton()->Bind(wxEVT_BUTTON, &MainFrame::OnConnectButton, this);
    batteryProfileChoice_->Bind(wxEVT_CHOICE, &MainFrame::OnBatteryProfileChanged, this);
    saveBatteryProfileButton_->Bind(wxEVT_BUTTON, &MainFrame::OnSaveBatteryProfile, this);
    chooseBatteryProfileButton_->Bind(wxEVT_BUTTON, &MainFrame::OnChooseBatteryProfile, this);
    clearBatteryProfilesButton_->Bind(wxEVT_BUTTON, &MainFrame::OnClearBatteryProfiles, this);
    chargeRadio_->Bind(wxEVT_RADIOBUTTON, &MainFrame::OnFunctionChanged, this);
    dischargeRadio_->Bind(wxEVT_RADIOBUTTON, &MainFrame::OnFunctionChanged, this);
    cycleRadio_->Bind(wxEVT_RADIOBUTTON, &MainFrame::OnFunctionChanged, this);
    cycleModeChoice_->Bind(wxEVT_CHOICE, &MainFrame::OnCycleModeChanged, this);
    backStepButton_->Bind(wxEVT_BUTTON, &MainFrame::OnBackStep, this);
    nextStepButton_->Bind(wxEVT_BUTTON, &MainFrame::OnNextStep, this);
    startButton_->Bind(wxEVT_BUTTON, &MainFrame::OnStartOperation, this);
    stopButton_->Bind(wxEVT_BUTTON, &MainFrame::OnStopOperation, this);
    exportButton_->Bind(wxEVT_BUTTON, &MainFrame::OnExportData, this);
}

void MainFrame::ConfigureBrandStyle() {
    startButton_->SetBackgroundColour(wxColour(29, 108, 201));
    startButton_->SetForegroundColour(*wxWHITE);
    stopButton_->SetBackgroundColour(wxColour(175, 43, 43));
    stopButton_->SetForegroundColour(*wxWHITE);
    exportButton_->SetBackgroundColour(wxColour(70, 78, 96));
    exportButton_->SetForegroundColour(*wxWHITE);

    stopButton_->Hide();
}

} // namespace batview::ui::frames
