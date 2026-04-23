#include "ui/frames/MainFrame.h"

#include <algorithm>
#include <wx/icon.h>
#include <wx/font.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stdpaths.h>
#include <wx/statline.h>
#include <wx/textctrl.h>

#include <filesystem>
#include <thread>
#include <vector>

#include "ui/panels/PlotPanel.h"

namespace batview::ui::frames {

namespace {

const wxColour kTerminalBackground(10, 14, 18);
const wxColour kTerminalForeground(116, 255, 153);

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

std::vector<std::filesystem::path> GetAssetDirectories() {
    std::vector<std::filesystem::path> directories;

    directories.emplace_back(BATVIEW_ASSET_DIR);
    directories.emplace_back(BATVIEW_SOURCE_ASSET_DIR);

    const wxStandardPaths& standardPaths = wxStandardPaths::Get();
    directories.emplace_back(standardPaths.GetResourcesDir().ToStdString());

    const std::filesystem::path executablePath(standardPaths.GetExecutablePath().ToStdString());
    if (!executablePath.empty()) {
        const std::filesystem::path executableDir = executablePath.parent_path();
        directories.push_back(executableDir);
        directories.push_back(executableDir / BATVIEW_INSTALL_ASSET_SUBDIR);
    }

    std::sort(directories.begin(), directories.end());
    directories.erase(std::unique(directories.begin(), directories.end()), directories.end());
    return directories;
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

} // namespace

MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1280, 780)),
      splashTimer_(this),
      wizardStep_(0),
      isConnected_(false),
      batteryChosen_(false),
      functionChosen_(false),
      operationActive_(false),
      isInfiniteRunning_(false),
      connectionInProgress_(false),
      workspaceSplitter_(nullptr),
      workflowSplitter_(nullptr),
      plotPanel_(nullptr),
      trafficLogCtrl_(nullptr) {
    SetAppIcon(*this);
    BuildLayout();
    ConfigureBrandStyle();
    BindEvents();
}

void MainFrame::SetViewModel(std::shared_ptr<batview::ui::viewmodels::MainViewModel> viewModel) {
    viewModel_ = viewModel;

    if (viewModel_) {
        viewModel_->SetTrafficObserver([this](bool outgoing, const std::string& line) {
            AppendTraffic(outgoing, line);
        });
    }

    if (plotPanel_) {
        plotPanel_->SetViewModel(viewModel_);
    }
}

void MainFrame::BuildLayout() {
    mainPanel_ = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    PrepareFlatPanel(mainPanel_);

    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    workspaceSplitter_ = new wxSplitterWindow(mainPanel_, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                              wxSP_LIVE_UPDATE | wxSP_NO_XP_THEME);
    workspaceSplitter_->SetMinimumPaneSize(260);

    auto* leftPane = new wxPanel(workspaceSplitter_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    PrepareFlatPanel(leftPane);

    auto* rightPane = new wxPanel(workspaceSplitter_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    PrepareFlatPanel(rightPane);

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

    for (const auto& assetDir : GetAssetDirectories()) {
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
    PrepareFlatPanel(workflowPage_);
    workflowSplitter_ = new wxSplitterWindow(workflowPage_, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                             wxSP_LIVE_UPDATE | wxSP_NO_XP_THEME);
    workflowSplitter_->SetMinimumPaneSize(160);
    workflowSplitter_->SetSashGravity(0.62);

    flowPanel_ = new wxPanel(workflowSplitter_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    PrepareFlatPanel(flowPanel_);

    connectionPanel_ = new batview::ui::panels::ConnectionPanel(flowPanel_);

    batteryLabel_ = new wxStaticText(flowPanel_, wxID_ANY, "Bateria");
    batteryChoice_ = new wxChoice(flowPanel_, wxID_ANY);
    batteryChoice_->Append("Li-ion");
    batteryChoice_->Append("LiPo");
    batteryChoice_->Append("NiMH");
    batteryChoice_->Append("LiFePO4");
    batteryChoice_->SetSelection(wxNOT_FOUND);

    functionPanel_ = new wxPanel(flowPanel_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    PrepareFlatPanel(functionPanel_);
    auto* functionLabel = new wxStaticText(functionPanel_, wxID_ANY, "Funcion");
    chargeRadio_ = new wxRadioButton(functionPanel_, wxID_ANY, "Carga", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    dischargeRadio_ = new wxRadioButton(functionPanel_, wxID_ANY, "Descarga");
    cycleRadio_ = new wxRadioButton(functionPanel_, wxID_ANY, "Ciclado");
    chargeRadio_->SetValue(false);
    dischargeRadio_->SetValue(false);
    cycleRadio_->SetValue(false);

    auto* functionRow = new wxBoxSizer(wxHORIZONTAL);
    functionRow->Add(chargeRadio_, 0, wxRIGHT, 14);
    functionRow->Add(dischargeRadio_, 0, wxRIGHT, 14);
    functionRow->Add(cycleRadio_, 0);

    auto* functionSizer = new wxBoxSizer(wxVERTICAL);
    functionSizer->Add(functionLabel, 0, wxBOTTOM, 4);
    functionSizer->Add(functionRow, 0, wxTOP, 2);
    functionPanel_->SetSizer(functionSizer);

    optionsPanel_ = new wxPanel(flowPanel_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    PrepareFlatPanel(optionsPanel_);
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
    flowSizer->Add(batteryLabel_, 0, wxBOTTOM, 4);
    flowSizer->Add(batteryChoice_, 0, wxEXPAND | wxBOTTOM, 10);
    flowSizer->Add(functionPanel_, 0, wxEXPAND | wxBOTTOM, 10);
    flowSizer->Add(optionsPanel_, 0, wxEXPAND | wxBOTTOM, 10);
    flowPanel_->SetSizer(flowSizer);

    auto* trafficPane = new wxPanel(workflowSplitter_, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    PrepareFlatPanel(trafficPane);
    auto* trafficSizer = new wxBoxSizer(wxVERTICAL);
    auto* trafficTitle = new wxStaticText(trafficPane, wxID_ANY, "Mensajes ESP32 / PC");
    StyleTerminalTitle(trafficTitle);
    trafficLogCtrl_ = new wxTextCtrl(trafficPane, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 220),
                                     wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 | wxTE_DONTWRAP | wxHSCROLL |
                                         wxBORDER_NONE);
    StyleTerminalTextBox(trafficLogCtrl_);
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
    batteryChoice_->Bind(wxEVT_CHOICE, &MainFrame::OnBatteryChanged, this);
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

void MainFrame::UpdateFlowVisibility() {
    const bool showBattery = isConnected_ && wizardStep_ >= 1;
    const bool showFunction = isConnected_ && batteryChosen_ && wizardStep_ >= 2;
    const bool showOptions = isConnected_ && batteryChosen_ && functionChosen_ && wizardStep_ >= 3;

    batteryLabel_->Show(showBattery);
    batteryChoice_->Show(showBattery);
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
    batteryChoice_->SetSelection(wxNOT_FOUND);
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

void MainFrame::OnBatteryChanged(wxCommandEvent& event) {
    (void)event;
    batteryChosen_ = batteryChoice_->GetSelection() != wxNOT_FOUND;
    if (!batteryChosen_) {
        functionChosen_ = false;
        operationActive_ = false;
    } else if (wizardStep_ < 2) {
        wizardStep_ = 2;
    }
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

    const int batteryTypeCode = GetSelectedBatteryTypeCode();
    const int functionCode = GetSelectedFunctionCode();

    if (!viewModel_->SendBatterySelection(batteryTypeCode, functionCode)) {
        ShowCommunicationFailure("No se recibio confirmacion del ESP32 para la seleccion de bateria.");
        return;
    }

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

int MainFrame::GetSelectedBatteryTypeCode() const {
    return batteryChoice_ ? batteryChoice_->GetSelection() + 1 : 1;
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
