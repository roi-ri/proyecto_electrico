#include "ui/panels/PlotPanel.h"

#ifdef BATVIEW_ENABLE_WX

#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dcmemory.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/mstream.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/window.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <map>
#include <set>

namespace batview::ui::panels {

namespace {

constexpr int kRefreshIntervalMs = 700;
constexpr double kMinZoomScale = 0.125;
constexpr double kMaxZoomScale = 64.0;
constexpr double kZoomStep = 1.5;
constexpr auto kPanRefreshInterval = std::chrono::milliseconds(60);

std::array<batview::core::services::PlotAxis, 5> kAxes = {
    batview::core::services::PlotAxis::TimestampMs,
    batview::core::services::PlotAxis::Voltage,
    batview::core::services::PlotAxis::Current,
    batview::core::services::PlotAxis::State,
    batview::core::services::PlotAxis::CompletedCycles
};

wxString ToWx(const std::string& value) {
    return wxString::FromUTF8(value.c_str());
}

std::string FormatPointSummary(const batview::core::models::Measurement& measurement) {
    return "Ultimo punto | t="
        + std::to_string(measurement.timestampMs)
        + " ms, Tension (V)=" + std::to_string(measurement.voltage)
        + ", Corriente (A)=" + std::to_string(measurement.current);
}

double ExtractAxisValue(const batview::core::models::Measurement& measurement,
                        batview::core::services::PlotAxis axis) {
    switch (axis) {
    case batview::core::services::PlotAxis::TimestampMs:
        return static_cast<double>(measurement.timestampMs) / 1000.0;
    case batview::core::services::PlotAxis::Voltage:
        return measurement.voltage;
    case batview::core::services::PlotAxis::Current:
        return measurement.current;
    case batview::core::services::PlotAxis::State:
        return static_cast<double>(measurement.state);
    case batview::core::services::PlotAxis::CompletedCycles:
        return static_cast<double>(measurement.completedCycles);
    }

    return 0.0;
}

struct AxisRange {
    double minValue = 0.0;
    double maxValue = 1.0;
};

struct StateInfo {
    int code = 0;
    const char* label = "";
    wxColour color;
};

StateInfo StateInfoForCode(int code) {
    switch (code) {
    case -1:
        return {-1, "Sin estado", wxColour(120, 130, 145)};
    case 0:
        return {0, "Descarga", wxColour(98, 166, 255)};
    case 1:
        return {1, "Carga", wxColour(90, 255, 180)};
    case 2:
        return {2, "Reposo", wxColour(255, 190, 112)};
    case 3:
        return {3, "Ciclado", wxColour(203, 126, 255)};
    case 4:
        return {4, "Finalizado", wxColour(120, 220, 120)};
    default:
        return {code, "Estado", wxColour(220, 220, 220)};
    }
}

std::vector<int> CollectStateCodes(const std::vector<batview::core::models::Measurement>& measurements) {
    std::set<int> codes {-1, 0, 1, 2, 3, 4};
    for (const auto& measurement : measurements) {
        codes.insert(measurement.state);
    }
    return {codes.begin(), codes.end()};
}

AxisRange ComputeAxisRange(const std::vector<double>& values) {
    if (values.empty()) {
        return {};
    }

    const auto [minIt, maxIt] = std::minmax_element(values.begin(), values.end());
    AxisRange range {*minIt, *maxIt};
    if (std::abs(range.maxValue - range.minValue) < 1e-9) {
        const double delta = range.minValue == 0.0 ? 1.0 : std::abs(range.minValue) * 0.1;
        range.minValue -= delta;
        range.maxValue += delta;
    }
    return range;
}

AxisRange ApplyZoom(AxisRange range, double zoomScale) {
    zoomScale = std::clamp(zoomScale, kMinZoomScale, kMaxZoomScale);
    const double center = (range.minValue + range.maxValue) / 2.0;
    const double halfSpan = (range.maxValue - range.minValue) / (2.0 * zoomScale);
    range.minValue = center - halfSpan;
    range.maxValue = center + halfSpan;
    return range;
}

AxisRange ApplyPan(AxisRange range, double panFraction) {
    const double span = range.maxValue - range.minValue;
    range.minValue += span * panFraction;
    range.maxValue += span * panFraction;
    return range;
}

int ProjectValue(double value, const AxisRange& range, int dstMin, int dstMax) {
    if (std::abs(range.maxValue - range.minValue) < 1e-9) {
        return (dstMin + dstMax) / 2;
    }

    const double ratio = (value - range.minValue) / (range.maxValue - range.minValue);
    return static_cast<int>(std::lround(dstMin + ratio * (dstMax - dstMin)));
}

wxString FormatStateTick(int code) {
    const auto info = StateInfoForCode(code);
    if (std::string(info.label) == "Estado") {
        return wxString::Format("Estado %d", code);
    }
    return wxString::Format("%s (%d)", info.label, code);
}

bool HasExplicitDischargeState(const std::vector<batview::core::models::Measurement>& measurements) {
    return std::any_of(measurements.begin(), measurements.end(), [](const auto& measurement) {
        return measurement.state == 0;
    });
}

bool IsDischargeSample(const batview::core::models::Measurement& measurement, bool hasExplicitDischargeState) {
    if (hasExplicitDischargeState) {
        return measurement.state == 0;
    }
    return true;
}

double ComputeMeasuredCapacityMah(const std::vector<batview::core::models::Measurement>& measurements,
                                  std::size_t endIndex) {
    if (measurements.size() < 2 || endIndex == 0) {
        return 0.0;
    }

    endIndex = std::min(endIndex, measurements.size() - 1);
    const bool hasExplicitDischargeState = HasExplicitDischargeState(measurements);
    double capacityMah = 0.0;
    for (std::size_t index = 1; index <= endIndex; ++index) {
        const auto& previous = measurements[index - 1];
        const auto& current = measurements[index];
        if (!IsDischargeSample(previous, hasExplicitDischargeState) &&
            !IsDischargeSample(current, hasExplicitDischargeState)) {
            continue;
        }

        const double deltaHours = std::abs(static_cast<double>(current.timestampMs) -
                                           static_cast<double>(previous.timestampMs)) /
            3600000.0;
        const double averageCurrentA = (std::abs(previous.current) + std::abs(current.current)) / 2.0;
        capacityMah += averageCurrentA * deltaHours * 1000.0;
    }
    return capacityMah;
}

double BatteryHealthPercent(double measuredCapacityMah, double factoryCapacityMah) {
    if (factoryCapacityMah <= 0.0) {
        return 0.0;
    }
    return std::clamp((measuredCapacityMah / factoryCapacityMah) * 100.0, 0.0, 100.0);
}

wxColour BatteryHealthColor(double healthPercent) {
    if (healthPercent >= 70.0) {
        return wxColour(90, 255, 180);
    }
    if (healthPercent >= 40.0) {
        return wxColour(255, 190, 112);
    }
    return wxColour(255, 105, 105);
}

} // namespace

struct PlotPanel::PlotWidgets {
    wxChoice* modeChoice = nullptr;
    wxStaticText* factoryCapacityLabel = nullptr;
    wxTextCtrl* factoryCapacityCtrl = nullptr;
    wxStaticText* xAxisLabel = nullptr;
    wxChoice* xAxisChoice = nullptr;
    wxStaticText* yAxisLabel = nullptr;
    wxChoice* yAxisChoice = nullptr;
    wxStaticBitmap* bitmap = nullptr;
    wxStaticText* status = nullptr;
    wxButton* exportButton = nullptr;
    wxButton* zoomXInButton = nullptr;
    wxButton* zoomXOutButton = nullptr;
    wxButton* zoomYInButton = nullptr;
    wxButton* zoomYOutButton = nullptr;
    wxButton* autoFitButton = nullptr;
    wxStaticText* zoomLabel = nullptr;
    std::string title;
    std::size_t lastPointCount = 0;
    double zoomScaleX = 1.0;
    double zoomScaleY = 1.0;
    double panX = 0.0;
    double panY = 0.0;
    bool isPanning = false;
    wxPoint lastPanPosition;
    std::chrono::steady_clock::time_point lastPanRefresh = std::chrono::steady_clock::now();
};

PlotPanel::PlotPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE),
      clearPlotsButton_(nullptr),
      plotsSplitter_(nullptr),
      refreshTimer_(this) {
    SetBackgroundStyle(wxBG_STYLE_SYSTEM);

    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    auto* headerSizer = new wxBoxSizer(wxHORIZONTAL);
    headerSizer->AddStretchSpacer(1);
    clearPlotsButton_ = new wxButton(this, wxID_ANY, "Limpiar graficos");
    headerSizer->Add(clearPlotsButton_, 0, wxALL, 8);
    mainSizer->Add(headerSizer, 0, wxEXPAND);

    plotsSplitter_ = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_NO_XP_THEME);
    plotsSplitter_->SetMinimumPaneSize(180);
    auto* firstPlotCard = BuildPlotCard(plotsSplitter_, "Grafico 1", 0);
    auto* secondPlotCard = BuildPlotCard(plotsSplitter_, "Grafico 2", 1);
    plotsSplitter_->SplitHorizontally(firstPlotCard, secondPlotCard, 420);
    plotsSplitter_->SetSashGravity(0.5);
    mainSizer->Add(plotsSplitter_, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    SetSizer(mainSizer);

    Bind(wxEVT_TIMER, &PlotPanel::OnRefreshTimer, this);
    clearPlotsButton_->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        ClearPlots();
    });
    refreshTimer_.Start(kRefreshIntervalMs);
}

void PlotPanel::SetViewModel(std::shared_ptr<batview::ui::viewmodels::MainViewModel> viewModel) {
    viewModel_ = std::move(viewModel);
    RefreshPlots();
}

wxPanel* PlotPanel::BuildPlotCard(wxWindow* parent,
                                  const std::string& title,
                                  int index) {
    auto widgets = std::make_unique<PlotWidgets>();
    widgets->title = title;

    auto* card = new wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
    card->SetBackgroundStyle(wxBG_STYLE_SYSTEM);

    auto* titleLabel = new wxStaticText(card, wxID_ANY, ToWx(title));
    wxFont titleFont = titleLabel->GetFont();
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleLabel->SetFont(titleFont);

    widgets->xAxisChoice = new wxChoice(card, wxID_ANY);
    widgets->yAxisChoice = new wxChoice(card, wxID_ANY);
    widgets->xAxisLabel = new wxStaticText(card, wxID_ANY, "X");
    widgets->yAxisLabel = new wxStaticText(card, wxID_ANY, "Y");
    if (index == 1) {
        widgets->modeChoice = new wxChoice(card, wxID_ANY);
        widgets->modeChoice->Append("Graficacion");
        widgets->modeChoice->Append("Vida util");
        widgets->modeChoice->SetSelection(0);
        widgets->factoryCapacityLabel = new wxStaticText(card, wxID_ANY, "Capacidad fabrica (mAh)");
        widgets->factoryCapacityCtrl = new wxTextCtrl(card, wxID_ANY, "2000");
        widgets->factoryCapacityCtrl->SetMinSize(wxSize(90, -1));
    }
    for (const auto axis : kAxes) {
        widgets->xAxisChoice->Append(ToWx(batview::core::services::PlotService::AxisLabel(axis)));
        widgets->yAxisChoice->Append(ToWx(batview::core::services::PlotService::AxisLabel(axis)));
    }

    widgets->xAxisChoice->SetSelection(index == 0 ? 0 : 0);
    widgets->yAxisChoice->SetSelection(index == 0 ? 1 : 2);

    widgets->bitmap = new wxStaticBitmap(card, wxID_ANY, wxBitmap(720, 300));
    widgets->bitmap->SetMinSize(wxSize(480, 300));
    widgets->status = new wxStaticText(card, wxID_ANY, "Esperando datos...");
    widgets->zoomLabel = new wxStaticText(card, wxID_ANY, "X 100% | Y 100%");
    widgets->zoomXOutButton = new wxButton(card, wxID_ANY, "X -");
    widgets->zoomXInButton = new wxButton(card, wxID_ANY, "X +");
    widgets->zoomYOutButton = new wxButton(card, wxID_ANY, "Y -");
    widgets->zoomYInButton = new wxButton(card, wxID_ANY, "Y +");
    widgets->autoFitButton = new wxButton(card, wxID_ANY, "Auto");
    widgets->exportButton = new wxButton(card, wxID_ANY, "Exportar grafico");
    widgets->zoomXOutButton->SetToolTip("Alejar eje X");
    widgets->zoomXInButton->SetToolTip("Acercar eje X");
    widgets->zoomYOutButton->SetToolTip("Alejar eje Y");
    widgets->zoomYInButton->SetToolTip("Acercar eje Y");
    widgets->autoFitButton->SetToolTip("Auto ajustar el rango del grafico");
    widgets->bitmap->SetToolTip("Arrastre dentro del grafico para navegar. Use la rueda para acercar o alejar ambos ejes.");

    auto refreshHandler = [this](wxCommandEvent&) { RefreshPlots(); };
    if (widgets->modeChoice) {
        widgets->modeChoice->Bind(wxEVT_CHOICE, [this, index](wxCommandEvent&) {
            auto* currentWidgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
            if (currentWidgets) {
                UpdatePlotModeControls(*currentWidgets);
            }
            RefreshPlot(index);
        });
    }
    if (widgets->factoryCapacityCtrl) {
        widgets->factoryCapacityCtrl->Bind(wxEVT_TEXT, [this, index](wxCommandEvent&) {
            RefreshPlot(index);
        });
    }
    widgets->xAxisChoice->Bind(wxEVT_CHOICE, refreshHandler);
    widgets->yAxisChoice->Bind(wxEVT_CHOICE, refreshHandler);
    widgets->exportButton->Bind(wxEVT_BUTTON, [this, index](wxCommandEvent&) {
        ExportPlot(index);
    });
    widgets->zoomXOutButton->Bind(wxEVT_BUTTON, [this, index](wxCommandEvent&) {
        auto* currentWidgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
        if (currentWidgets) {
            SetPlotZoom(index, currentWidgets->zoomScaleX / kZoomStep, currentWidgets->zoomScaleY);
        }
    });
    widgets->zoomXInButton->Bind(wxEVT_BUTTON, [this, index](wxCommandEvent&) {
        auto* currentWidgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
        if (currentWidgets) {
            SetPlotZoom(index, currentWidgets->zoomScaleX * kZoomStep, currentWidgets->zoomScaleY);
        }
    });
    widgets->zoomYOutButton->Bind(wxEVT_BUTTON, [this, index](wxCommandEvent&) {
        auto* currentWidgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
        if (currentWidgets) {
            SetPlotZoom(index, currentWidgets->zoomScaleX, currentWidgets->zoomScaleY / kZoomStep);
        }
    });
    widgets->zoomYInButton->Bind(wxEVT_BUTTON, [this, index](wxCommandEvent&) {
        auto* currentWidgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
        if (currentWidgets) {
            SetPlotZoom(index, currentWidgets->zoomScaleX, currentWidgets->zoomScaleY * kZoomStep);
        }
    });
    widgets->autoFitButton->Bind(wxEVT_BUTTON, [this, index](wxCommandEvent&) {
        ResetPlotView(index);
    });
    widgets->bitmap->Bind(wxEVT_LEFT_DOWN, [this, index](wxMouseEvent& event) {
        BeginPlotPan(index, event.GetPosition());
    });
    widgets->bitmap->Bind(wxEVT_MOTION, [this, index](wxMouseEvent& event) {
        if (event.Dragging() && event.LeftIsDown()) {
            ContinuePlotPan(index, event.GetPosition());
        }
    });
    widgets->bitmap->Bind(wxEVT_LEFT_UP, [this, index](wxMouseEvent&) {
        EndPlotPan(index);
    });
    widgets->bitmap->Bind(wxEVT_LEAVE_WINDOW, [this, index](wxMouseEvent&) {
        EndPlotPan(index);
    });
    widgets->bitmap->Bind(wxEVT_MOUSEWHEEL, [this, index](wxMouseEvent& event) {
        auto* currentWidgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
        if (currentWidgets && !IsBatteryHealthMode(*currentWidgets)) {
            const double factor = event.GetWheelRotation() > 0 ? kZoomStep : 1.0 / kZoomStep;
            SetPlotZoom(index, currentWidgets->zoomScaleX * factor, currentWidgets->zoomScaleY * factor);
        }
    });

    auto* controlsSizer = new wxBoxSizer(wxHORIZONTAL);
    if (widgets->modeChoice) {
        controlsSizer->Add(new wxStaticText(card, wxID_ANY, "Modo"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
        controlsSizer->Add(widgets->modeChoice, 0, wxRIGHT, 12);
        controlsSizer->Add(widgets->factoryCapacityLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
        controlsSizer->Add(widgets->factoryCapacityCtrl, 0, wxRIGHT, 12);
    }
    controlsSizer->Add(widgets->xAxisLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    controlsSizer->Add(widgets->xAxisChoice, 1, wxRIGHT, 12);
    controlsSizer->Add(widgets->yAxisLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    controlsSizer->Add(widgets->yAxisChoice, 1, wxRIGHT, 12);
    controlsSizer->Add(widgets->exportButton, 0);

    auto* zoomSizer = new wxBoxSizer(wxHORIZONTAL);
    zoomSizer->Add(new wxStaticText(card, wxID_ANY, "Zoom"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    zoomSizer->Add(widgets->zoomXOutButton, 0, wxRIGHT, 6);
    zoomSizer->Add(widgets->zoomXInButton, 0, wxRIGHT, 10);
    zoomSizer->Add(widgets->zoomYOutButton, 0, wxRIGHT, 6);
    zoomSizer->Add(widgets->zoomYInButton, 0, wxRIGHT, 10);
    zoomSizer->Add(widgets->autoFitButton, 0, wxRIGHT, 8);
    zoomSizer->Add(widgets->zoomLabel, 0, wxALIGN_CENTER_VERTICAL);

    auto* cardSizer = new wxBoxSizer(wxVERTICAL);
    cardSizer->Add(titleLabel, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    cardSizer->Add(controlsSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
    cardSizer->Add(zoomSizer, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    cardSizer->Add(widgets->bitmap, 1, wxEXPAND | wxALL, 10);
    cardSizer->Add(widgets->status, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
    card->SetSizer(cardSizer);
    UpdatePlotModeControls(*widgets);

    if (index == 0) {
        primaryPlot_ = std::move(widgets);
    } else {
        secondaryPlot_ = std::move(widgets);
    }

    return card;
}

void PlotPanel::RefreshPlots() {
    RefreshPlot(0);
    RefreshPlot(1);
}

wxBitmap PlotPanel::RenderPlotBitmap(const std::vector<batview::core::models::Measurement>& measurements,
                                     batview::core::services::PlotAxis xAxis,
                                     batview::core::services::PlotAxis yAxis,
                                     const wxSize& requestedSize,
                                     double zoomScaleX,
                                     double zoomScaleY,
                                     double panX,
                                     double panY,
                                     const std::string& title,
                                     std::string& outError,
                                     std::size_t& outPointCount) const {
    outError.clear();
    outPointCount = measurements.size();

    if (measurements.empty()) {
        outError = "Sin datos para graficar.";
        return wxBitmap();
    }

    const int width = std::max(requestedSize.GetWidth(), 480);
    const int height = std::max(requestedSize.GetHeight(), 300);
    wxBitmap bitmap(width, height, 32);
    wxMemoryDC dc(bitmap);

    dc.SetBackground(wxBrush(wxColour(13, 18, 24)));
    dc.Clear();

    const int marginLeft = 72;
    const int marginRight = 20;
    const int marginTop = 34;
    const int marginBottom = 46;
    const wxRect plotRect(marginLeft,
                          marginTop,
                          std::max(width - marginLeft - marginRight, 40),
                          std::max(height - marginTop - marginBottom, 40));

    dc.SetPen(wxPen(wxColour(55, 64, 78), 1));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(0, 0, width, height);
    dc.DrawRectangle(plotRect);

    std::vector<double> xValues;
    std::vector<double> yValues;
    xValues.reserve(measurements.size());
    yValues.reserve(measurements.size());
    for (const auto& measurement : measurements) {
        xValues.push_back(ExtractAxisValue(measurement, xAxis));
        yValues.push_back(ExtractAxisValue(measurement, yAxis));
    }

    const bool categoricalStatePlot = yAxis == batview::core::services::PlotAxis::State;
    const AxisRange xRange = ApplyPan(ApplyZoom(ComputeAxisRange(xValues), zoomScaleX), panX);
    const AxisRange yRange = categoricalStatePlot
        ? AxisRange {-0.5, 4.5}
        : ApplyPan(ApplyZoom(ComputeAxisRange(yValues), zoomScaleY), panY);

    dc.SetFont(GetFont());
    dc.SetTextForeground(wxColour(194, 202, 215));
    for (int index = 0; index < 5; ++index) {
        const double fraction = static_cast<double>(index) / 4.0;
        const int x = plotRect.GetLeft() + static_cast<int>(std::lround(fraction * plotRect.GetWidth()));
        dc.SetPen(wxPen(wxColour(35, 43, 52), 1));
        dc.DrawLine(x, plotRect.GetTop(), x, plotRect.GetBottom());

        const double xTick = xRange.minValue + (xRange.maxValue - xRange.minValue) * fraction;
        dc.DrawText(wxString::Format("%.2f", xTick), x - 18, plotRect.GetBottom() + 8);
    }

    std::vector<int> stateCodes;
    std::map<int, int> stateRows;
    if (categoricalStatePlot) {
        stateCodes = CollectStateCodes(measurements);
        const int rowCount = std::max(static_cast<int>(stateCodes.size()), 1);
        const int rowHeight = std::max(plotRect.GetHeight() / rowCount, 1);

        for (int row = 0; row < rowCount; ++row) {
            const int code = stateCodes[static_cast<std::size_t>(row)];
            stateRows[code] = row;
            const auto info = StateInfoForCode(code);
            const int top = plotRect.GetTop() + row * rowHeight;
            const int bottom = row == rowCount - 1 ? plotRect.GetBottom() : top + rowHeight;
            const wxColour bandColor(
                std::max(info.color.Red() / 6, 18),
                std::max(info.color.Green() / 6, 18),
                std::max(info.color.Blue() / 6, 18));

            dc.SetPen(wxPen(wxColour(35, 43, 52), 1));
            dc.SetBrush(wxBrush(bandColor));
            dc.DrawRectangle(plotRect.GetLeft(), top, plotRect.GetWidth(), std::max(bottom - top, 1));
            dc.SetTextForeground(info.color);
            dc.DrawText(FormatStateTick(code), 8, top + 4);
        }
    } else {
        for (int index = 0; index < 5; ++index) {
            const double fraction = static_cast<double>(index) / 4.0;
            const int y = plotRect.GetBottom() - static_cast<int>(std::lround(fraction * plotRect.GetHeight()));
            dc.SetPen(wxPen(wxColour(35, 43, 52), 1));
            dc.DrawLine(plotRect.GetLeft(), y, plotRect.GetRight(), y);

            const double yTick = yRange.minValue + (yRange.maxValue - yRange.minValue) * fraction;
            dc.SetTextForeground(wxColour(194, 202, 215));
            dc.DrawText(wxString::Format("%.2f", yTick), 8, y - 7);
        }
    }

    dc.SetTextForeground(wxColour(229, 236, 246));
    dc.DrawText(ToWx(title), plotRect.GetLeft(), 8);
    dc.SetTextForeground(wxColour(146, 200, 255));
    dc.DrawText(ToWx(batview::core::services::PlotService::AxisLabel(xAxis)), plotRect.GetLeft(), height - 24);
    dc.SetTextForeground(wxColour(255, 190, 112));
    dc.DrawText(ToWx(batview::core::services::PlotService::AxisLabel(yAxis)), 8, 8);

    dc.SetPen(wxPen(wxColour(90, 255, 180), 1));
    dc.SetBrush(wxBrush(wxColour(90, 255, 180)));

    std::vector<wxPoint> points;
    points.reserve(measurements.size());
    for (std::size_t index = 0; index < measurements.size(); ++index) {
        const int px = ProjectValue(xValues[index], xRange, plotRect.GetLeft(), plotRect.GetRight());
        int py = ProjectValue(yValues[index], yRange, plotRect.GetBottom(), plotRect.GetTop());
        if (categoricalStatePlot) {
            const int rowCount = std::max(static_cast<int>(stateCodes.size()), 1);
            const int rowHeight = std::max(plotRect.GetHeight() / rowCount, 1);
            const auto rowIt = stateRows.find(measurements[index].state);
            const int row = rowIt == stateRows.end() ? 0 : rowIt->second;
            const int top = plotRect.GetTop() + row * rowHeight;
            const int bottom = row == rowCount - 1 ? plotRect.GetBottom() : top + rowHeight;
            py = (top + bottom) / 2;
        }
        points.emplace_back(px, py);
    }

    dc.SetClippingRegion(plotRect);
    if (categoricalStatePlot) {
        if (points.size() > 1) {
            dc.SetPen(wxPen(wxColour(180, 225, 255), 1, wxPENSTYLE_DOT));
            dc.DrawLines(static_cast<int>(points.size()), points.data());
        }

        for (std::size_t index = 0; index < points.size(); ++index) {
            const auto info = StateInfoForCode(measurements[index].state);
            dc.SetPen(wxPen(info.color, 1));
            dc.SetBrush(wxBrush(info.color));
            dc.DrawCircle(points[index], 4);
        }
    } else if (points.size() == 1) {
        dc.DrawCircle(points.front(), 3);
    } else {
        dc.DrawLines(static_cast<int>(points.size()), points.data());
        dc.DrawCircle(points.back(), 3);
    }
    dc.DestroyClippingRegion();

    dc.SelectObject(wxNullBitmap);
    return bitmap;
}

wxBitmap PlotPanel::RenderBatteryHealthBitmap(
    const std::vector<batview::core::models::Measurement>& measurements,
    const wxSize& requestedSize,
    double factoryCapacityMah,
    std::string& outError,
    std::size_t& outPointCount) const {
    outError.clear();
    outPointCount = measurements.size();

    if (measurements.empty()) {
        outError = "Sin datos para calcular vida util.";
        return wxBitmap();
    }
    if (factoryCapacityMah <= 0.0) {
        outError = "Ingrese una capacidad de fabrica valida en mAh.";
        return wxBitmap();
    }

    const int width = std::max(requestedSize.GetWidth(), 480);
    const int height = std::max(requestedSize.GetHeight(), 300);
    wxBitmap bitmap(width, height, 32);
    wxMemoryDC dc(bitmap);

    dc.SetBackground(wxBrush(wxColour(13, 18, 24)));
    dc.Clear();
    dc.SetFont(GetFont());

    const double measuredCapacityMah = ComputeMeasuredCapacityMah(measurements, measurements.size() - 1);
    const double healthPercent = BatteryHealthPercent(measuredCapacityMah, factoryCapacityMah);
    const wxColour healthColor = BatteryHealthColor(healthPercent);

    dc.SetPen(wxPen(wxColour(55, 64, 78), 1));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(0, 0, width, height);

    dc.SetTextForeground(wxColour(229, 236, 246));
    dc.DrawText("Estado de Salud (SOH)", 24, 20);
    dc.SetTextForeground(wxColour(194, 202, 215));
    dc.DrawText("Capacidad actual / capacidad de fabrica x 100", 24, 46);

    wxFont percentFont = GetFont();
    percentFont.SetPointSize(percentFont.GetPointSize() + 20);
    percentFont.SetWeight(wxFONTWEIGHT_BOLD);
    dc.SetFont(percentFont);
    dc.SetTextForeground(healthColor);
    dc.DrawText(wxString::Format("%.1f%%", healthPercent), 24, 82);

    dc.SetFont(GetFont());
    dc.SetTextForeground(wxColour(194, 202, 215));
    dc.DrawText(wxString::Format("Capacidad actual: %.1f mAh", measuredCapacityMah), 28, 150);
    dc.DrawText(wxString::Format("Capacidad de fabrica: %.1f mAh", factoryCapacityMah), 28, 174);

    const int barLeft = 28;
    const int barTop = 214;
    const int barWidth = std::max(width - 56, 80);
    const int barHeight = 32;
    const int fillWidth = static_cast<int>(std::lround(barWidth * healthPercent / 100.0));
    dc.SetPen(wxPen(wxColour(70, 82, 98), 1));
    dc.SetBrush(wxBrush(wxColour(31, 39, 49)));
    dc.DrawRoundedRectangle(barLeft, barTop, barWidth, barHeight, 6);
    dc.SetPen(wxPen(healthColor, 1));
    dc.SetBrush(wxBrush(healthColor));
    dc.DrawRoundedRectangle(barLeft, barTop, std::max(fillWidth, 2), barHeight, 6);

    const int plotLeft = 28;
    const int plotTop = std::min(barTop + 64, height - 72);
    const int plotWidth = std::max(width - 56, 80);
    const int plotHeight = std::max(height - plotTop - 28, 40);
    dc.SetPen(wxPen(wxColour(55, 64, 78), 1));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(plotLeft, plotTop, plotWidth, plotHeight);

    std::vector<wxPoint> points;
    points.reserve(measurements.size());
    const double firstTimestamp = static_cast<double>(measurements.front().timestampMs);
    const double lastTimestamp = static_cast<double>(measurements.back().timestampMs);
    const double totalSeconds = std::max((lastTimestamp - firstTimestamp) / 1000.0, 1.0);
    for (std::size_t index = 0; index < measurements.size(); ++index) {
        const auto& measurement = measurements[index];
        const double seconds = (static_cast<double>(measurement.timestampMs) - firstTimestamp) / 1000.0;
        const double sampleCapacityMah = ComputeMeasuredCapacityMah(measurements, index);
        const double sampleHealth = BatteryHealthPercent(sampleCapacityMah, factoryCapacityMah);
        const int x = ProjectValue(seconds, {0.0, totalSeconds}, plotLeft, plotLeft + plotWidth);
        const int y = ProjectValue(sampleHealth, {0.0, 100.0}, plotTop + plotHeight, plotTop);
        points.emplace_back(x, y);
    }

    dc.SetClippingRegion(wxRect(plotLeft, plotTop, plotWidth, plotHeight));
    dc.SetPen(wxPen(healthColor, 2));
    if (points.size() == 1) {
        dc.SetBrush(wxBrush(healthColor));
        dc.DrawCircle(points.front(), 3);
    } else {
        dc.DrawLines(static_cast<int>(points.size()), points.data());
    }
    dc.DestroyClippingRegion();

    dc.SetTextForeground(wxColour(146, 200, 255));
    dc.DrawText("Tiempo", plotLeft, plotTop + plotHeight + 6);
    dc.SetTextForeground(wxColour(255, 190, 112));
    dc.DrawText("Salud (%)", plotLeft + 6, plotTop + 6);

    dc.SelectObject(wxNullBitmap);
    return bitmap;
}

void PlotPanel::RefreshPlot(int index) {
    auto* widgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
    if (widgets == nullptr || viewModel_ == nullptr) {
        return;
    }

    const auto measurements = viewModel_->GetMeasurements();
    std::string error;
    std::size_t pointCount = 0;
    wxBitmap bitmap;
    if (IsBatteryHealthMode(*widgets)) {
        double factoryCapacityMah = 0.0;
        if (widgets->factoryCapacityCtrl) {
            widgets->factoryCapacityCtrl->GetValue().ToDouble(&factoryCapacityMah);
        }
        bitmap = RenderBatteryHealthBitmap(measurements,
                                           widgets->bitmap->GetSize(),
                                           factoryCapacityMah,
                                           error,
                                           pointCount);
    } else {
        const auto xAxis = kAxes[static_cast<std::size_t>(widgets->xAxisChoice->GetSelection())];
        const auto yAxis = kAxes[static_cast<std::size_t>(widgets->yAxisChoice->GetSelection())];
        bitmap = RenderPlotBitmap(measurements,
                                  xAxis,
                                  yAxis,
                                  widgets->bitmap->GetSize(),
                                  widgets->zoomScaleX,
                                  widgets->zoomScaleY,
                                  widgets->panX,
                                  widgets->panY,
                                  widgets->title,
                                  error,
                                  pointCount);
    }

    if (!bitmap.IsOk()) {
        ResetPlotCard(*widgets);
        widgets->status->SetLabel(error.empty() ? "Sin datos para graficar." : ToWx(error));
        return;
    }

    widgets->bitmap->SetBitmap(bitmap);
    widgets->lastPointCount = pointCount;
    widgets->zoomLabel->SetLabel(wxString::Format("X %.0f%% | Y %.0f%%",
                                                  widgets->zoomScaleX * 100.0,
                                                  widgets->zoomScaleY * 100.0));
    if (IsBatteryHealthMode(*widgets)) {
        double factoryCapacityMah = 0.0;
        if (widgets->factoryCapacityCtrl) {
            widgets->factoryCapacityCtrl->GetValue().ToDouble(&factoryCapacityMah);
        }
        const double measuredCapacityMah = ComputeMeasuredCapacityMah(measurements, measurements.size() - 1);
        widgets->status->SetLabel(wxString::Format("SOH: %.1f%% | Capacidad actual: %.1f mAh | Fabrica: %.1f mAh",
                                                   BatteryHealthPercent(measuredCapacityMah, factoryCapacityMah),
                                                   measuredCapacityMah,
                                                   factoryCapacityMah));
        return;
    }

    if (!measurements.empty()) {
        widgets->status->SetLabel("Puntos graficados: "
                                  + std::to_string(pointCount)
                                  + " | "
                                  + FormatPointSummary(measurements.back()));
    }
}

bool PlotPanel::IsBatteryHealthMode(const PlotWidgets& widgets) const {
    return widgets.modeChoice != nullptr && widgets.modeChoice->GetSelection() == 1;
}

void PlotPanel::UpdatePlotModeControls(PlotWidgets& widgets) {
    const bool graphMode = !IsBatteryHealthMode(widgets);
    if (widgets.xAxisLabel) {
        widgets.xAxisLabel->Show(graphMode);
    }
    if (widgets.xAxisChoice) {
        widgets.xAxisChoice->Show(graphMode);
        widgets.xAxisChoice->Enable(graphMode);
    }
    if (widgets.yAxisLabel) {
        widgets.yAxisLabel->Show(graphMode);
    }
    if (widgets.yAxisChoice) {
        widgets.yAxisChoice->Show(graphMode);
        widgets.yAxisChoice->Enable(graphMode);
    }
    if (widgets.zoomXOutButton) {
        widgets.zoomXOutButton->Enable(graphMode);
    }
    if (widgets.zoomXInButton) {
        widgets.zoomXInButton->Enable(graphMode);
    }
    if (widgets.zoomYOutButton) {
        widgets.zoomYOutButton->Enable(graphMode);
    }
    if (widgets.zoomYInButton) {
        widgets.zoomYInButton->Enable(graphMode);
    }
    if (widgets.autoFitButton) {
        widgets.autoFitButton->Enable(graphMode);
    }
    if (widgets.factoryCapacityLabel) {
        widgets.factoryCapacityLabel->Show(!graphMode);
    }
    if (widgets.factoryCapacityCtrl) {
        widgets.factoryCapacityCtrl->Show(!graphMode);
    }
    if (widgets.factoryCapacityCtrl && widgets.factoryCapacityCtrl->GetParent()) {
        widgets.factoryCapacityCtrl->GetParent()->Layout();
    }
}

void PlotPanel::SetPlotZoom(int index, double zoomScaleX, double zoomScaleY) {
    auto* widgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
    if (!widgets) {
        return;
    }

    widgets->zoomScaleX = std::clamp(zoomScaleX, kMinZoomScale, kMaxZoomScale);
    widgets->zoomScaleY = std::clamp(zoomScaleY, kMinZoomScale, kMaxZoomScale);
    RefreshPlot(index);
}

void PlotPanel::ResetPlotView(int index) {
    auto* widgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
    if (!widgets) {
        return;
    }

    widgets->zoomScaleX = 1.0;
    widgets->zoomScaleY = 1.0;
    widgets->panX = 0.0;
    widgets->panY = 0.0;
    widgets->isPanning = false;
    RefreshPlot(index);
}

void PlotPanel::BeginPlotPan(int index, const wxPoint& position) {
    auto* widgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
    if (!widgets) {
        return;
    }
    if (IsBatteryHealthMode(*widgets)) {
        return;
    }

    widgets->isPanning = true;
    widgets->lastPanPosition = position;
    widgets->lastPanRefresh = std::chrono::steady_clock::now();
    if (!widgets->bitmap->HasCapture()) {
        widgets->bitmap->CaptureMouse();
    }
    widgets->bitmap->SetCursor(wxCursor(wxCURSOR_HAND));
}

void PlotPanel::ContinuePlotPan(int index, const wxPoint& position) {
    auto* widgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
    if (!widgets || !widgets->isPanning) {
        return;
    }

    const wxSize size = widgets->bitmap->GetSize();
    const int width = std::max(size.GetWidth(), 1);
    const int height = std::max(size.GetHeight(), 1);
    const int dx = position.x - widgets->lastPanPosition.x;
    const int dy = position.y - widgets->lastPanPosition.y;

    widgets->panX -= static_cast<double>(dx) / static_cast<double>(width);
    widgets->panY += static_cast<double>(dy) / static_cast<double>(height);
    widgets->lastPanPosition = position;

    const auto now = std::chrono::steady_clock::now();
    if (now - widgets->lastPanRefresh >= kPanRefreshInterval) {
        widgets->lastPanRefresh = now;
        RefreshPlot(index);
    }
}

void PlotPanel::EndPlotPan(int index) {
    auto* widgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
    if (!widgets || !widgets->isPanning) {
        return;
    }

    widgets->isPanning = false;
    if (widgets->bitmap->HasCapture()) {
        widgets->bitmap->ReleaseMouse();
    }
    widgets->bitmap->SetCursor(wxNullCursor);
    RefreshPlot(index);
}

void PlotPanel::ClearPlots() {
    if (viewModel_) {
        viewModel_->ClearPlotData();
    }

    if (primaryPlot_) {
        ResetPlotCard(*primaryPlot_);
    }
    if (secondaryPlot_) {
        ResetPlotCard(*secondaryPlot_);
    }
}

void PlotPanel::ResetPlotCard(PlotWidgets& widgets) {
    widgets.bitmap->SetBitmap(wxBitmap(720, 300));
    widgets.lastPointCount = 0;
    widgets.zoomScaleX = 1.0;
    widgets.zoomScaleY = 1.0;
    widgets.panX = 0.0;
    widgets.panY = 0.0;
    widgets.isPanning = false;
    if (widgets.zoomLabel) {
        widgets.zoomLabel->SetLabel("X 100% | Y 100%");
    }
    widgets.status->SetLabel("Sin datos para graficar.");
}

bool PlotPanel::HasActivePan() const {
    return (primaryPlot_ && primaryPlot_->isPanning) || (secondaryPlot_ && secondaryPlot_->isPanning);
}

bool PlotPanel::ExportPlotImage(const std::string& filePath, int index, std::string& outError) const {
    auto* widgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
    if (widgets == nullptr || viewModel_ == nullptr) {
        outError = "No hay grafico disponible para exportar.";
        return false;
    }

    if (IsBatteryHealthMode(*widgets)) {
        std::size_t pointCount = 0;
        double factoryCapacityMah = 0.0;
        if (widgets->factoryCapacityCtrl) {
            widgets->factoryCapacityCtrl->GetValue().ToDouble(&factoryCapacityMah);
        }
        wxBitmap bitmap = RenderBatteryHealthBitmap(viewModel_->GetMeasurements(),
                                                    widgets->bitmap->GetSize(),
                                                    factoryCapacityMah,
                                                    outError,
                                                    pointCount);
        if (!bitmap.IsOk()) {
            return false;
        }

        wxImage image = bitmap.ConvertToImage();
        if (!image.IsOk()) {
            outError = "La imagen generada de vida util no es valida.";
            return false;
        }

        if (!image.SaveFile(filePath, wxBITMAP_TYPE_PNG)) {
            outError = "No se pudo guardar la imagen PNG de vida util.";
            return false;
        }

        return true;
    }

    const auto xAxis = kAxes[static_cast<std::size_t>(widgets->xAxisChoice->GetSelection())];
    const auto yAxis = kAxes[static_cast<std::size_t>(widgets->yAxisChoice->GetSelection())];
    std::size_t pointCount = 0;
    wxBitmap bitmap = RenderPlotBitmap(viewModel_->GetMeasurements(),
                                       xAxis,
                                       yAxis,
                                       widgets->bitmap->GetSize(),
                                       widgets->zoomScaleX,
                                       widgets->zoomScaleY,
                                       widgets->panX,
                                       widgets->panY,
                                       widgets->title,
                                       outError,
                                       pointCount);
    if (!bitmap.IsOk()) {
        return false;
    }

    wxImage image = bitmap.ConvertToImage();
    if (!image.IsOk()) {
        outError = "La imagen generada del gráfico no es válida.";
        return false;
    }

    if (!image.SaveFile(filePath, wxBITMAP_TYPE_PNG)) {
        outError = "No se pudo guardar la imagen PNG del gráfico.";
        return false;
    }

    return true;
}

void PlotPanel::ExportPlot(int index) {
    auto* widgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
    if (widgets == nullptr || viewModel_ == nullptr) {
        return;
    }

    wxFileDialog dialog(this,
                        "Exportar datos del grafico",
                        "",
                        index == 0 ? "plot_1.csv" : "plot_2.csv",
                        "CSV (*.csv)|*.csv|MAT (*.mat)|*.mat|XLSX (*.xlsx)|*.xlsx|PNG (*.png)|*.png",
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dialog.ShowModal() != wxID_OK) {
        return;
    }

    std::string error;
    if (dialog.GetFilterIndex() == 3) {
        if (!ExportPlotImage(dialog.GetPath().ToStdString(), index, error)) {
            wxMessageBox(ToWx(error), "batView", wxICON_ERROR | wxOK, this);
            return;
        }

        wxMessageBox("Grafico exportado correctamente.", "batView", wxICON_INFORMATION | wxOK, this);
        return;
    }

    std::string format = "CSV";
    switch (dialog.GetFilterIndex()) {
    case 1:
        format = "MAT";
        break;
    case 2:
        format = "XLSX";
        break;
    default:
        break;
    }

    const auto xAxis = kAxes[static_cast<std::size_t>(widgets->xAxisChoice->GetSelection())];
    const auto yAxis = kAxes[static_cast<std::size_t>(widgets->yAxisChoice->GetSelection())];
    if (!viewModel_->ExportPlotData(dialog.GetPath().ToStdString(), format, xAxis, yAxis, error)) {
        wxMessageBox(ToWx(error.empty() ? "No se pudo exportar el gráfico." : error),
                     "batView",
                     wxICON_ERROR | wxOK,
                     this);
        return;
    }

    wxMessageBox("Grafico exportado correctamente.", "batView", wxICON_INFORMATION | wxOK, this);
}

void PlotPanel::OnRefreshTimer(wxTimerEvent& event) {
    (void)event;
    if (HasActivePan()) {
        return;
    }
    RefreshPlots();
}

} // namespace batview::ui::panels

#endif
