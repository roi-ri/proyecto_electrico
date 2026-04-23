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
#include <wx/statbmp.h>
#include <wx/stattext.h>

#include <array>
#include <cmath>

namespace batview::ui::panels {

namespace {

constexpr int kRefreshIntervalMs = 700;

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
        + " ms, V=" + std::to_string(measurement.voltage)
        + ", I=" + std::to_string(measurement.current);
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

int ProjectValue(double value, const AxisRange& range, int dstMin, int dstMax) {
    if (std::abs(range.maxValue - range.minValue) < 1e-9) {
        return (dstMin + dstMax) / 2;
    }

    const double ratio = (value - range.minValue) / (range.maxValue - range.minValue);
    return static_cast<int>(std::lround(dstMin + ratio * (dstMax - dstMin)));
}

} // namespace

struct PlotPanel::PlotWidgets {
    wxChoice* xAxisChoice = nullptr;
    wxChoice* yAxisChoice = nullptr;
    wxStaticBitmap* bitmap = nullptr;
    wxStaticText* status = nullptr;
    wxButton* exportButton = nullptr;
    std::string title;
    std::size_t lastPointCount = 0;
};

PlotPanel::PlotPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE),
      clearPlotsButton_(nullptr),
      refreshTimer_(this) {
    SetBackgroundStyle(wxBG_STYLE_SYSTEM);

    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    auto* headerSizer = new wxBoxSizer(wxHORIZONTAL);
    headerSizer->AddStretchSpacer(1);
    clearPlotsButton_ = new wxButton(this, wxID_ANY, "Limpiar graficos");
    headerSizer->Add(clearPlotsButton_, 0, wxALL, 8);
    mainSizer->Add(headerSizer, 0, wxEXPAND);
    BuildPlotCard(this, mainSizer, "Grafico 1", 0);
    BuildPlotCard(this, mainSizer, "Grafico 2", 1);
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

void PlotPanel::BuildPlotCard(wxWindow* parent,
                              wxSizer* parentSizer,
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
    for (const auto axis : kAxes) {
        widgets->xAxisChoice->Append(ToWx(batview::core::services::PlotService::AxisLabel(axis)));
        widgets->yAxisChoice->Append(ToWx(batview::core::services::PlotService::AxisLabel(axis)));
    }

    widgets->xAxisChoice->SetSelection(index == 0 ? 0 : 0);
    widgets->yAxisChoice->SetSelection(index == 0 ? 1 : 2);

    widgets->bitmap = new wxStaticBitmap(card, wxID_ANY, wxBitmap(720, 260));
    widgets->status = new wxStaticText(card, wxID_ANY, "Esperando datos...");
    widgets->exportButton = new wxButton(card, wxID_ANY, "Exportar grafico");

    auto refreshHandler = [this](wxCommandEvent&) { RefreshPlots(); };
    widgets->xAxisChoice->Bind(wxEVT_CHOICE, refreshHandler);
    widgets->yAxisChoice->Bind(wxEVT_CHOICE, refreshHandler);
    widgets->exportButton->Bind(wxEVT_BUTTON, [this, index](wxCommandEvent&) {
        ExportPlot(index);
    });

    auto* controlsSizer = new wxBoxSizer(wxHORIZONTAL);
    controlsSizer->Add(new wxStaticText(card, wxID_ANY, "X"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    controlsSizer->Add(widgets->xAxisChoice, 1, wxRIGHT, 12);
    controlsSizer->Add(new wxStaticText(card, wxID_ANY, "Y"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    controlsSizer->Add(widgets->yAxisChoice, 1, wxRIGHT, 12);
    controlsSizer->Add(widgets->exportButton, 0);

    auto* cardSizer = new wxBoxSizer(wxVERTICAL);
    cardSizer->Add(titleLabel, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    cardSizer->Add(controlsSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
    cardSizer->Add(widgets->bitmap, 1, wxEXPAND | wxALL, 10);
    cardSizer->Add(widgets->status, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
    card->SetSizer(cardSizer);

    parentSizer->Add(card, 1, wxEXPAND | wxALL, 8);

    if (index == 0) {
        primaryPlot_ = std::move(widgets);
    } else {
        secondaryPlot_ = std::move(widgets);
    }
}

void PlotPanel::RefreshPlots() {
    RefreshPlot(0);
    RefreshPlot(1);
}

wxBitmap PlotPanel::RenderPlotBitmap(const std::vector<batview::core::models::Measurement>& measurements,
                                     batview::core::services::PlotAxis xAxis,
                                     batview::core::services::PlotAxis yAxis,
                                     const wxSize& requestedSize,
                                     const std::string& title,
                                     std::string& outError,
                                     std::size_t& outPointCount) const {
    outError.clear();
    outPointCount = measurements.size();

    if (measurements.empty()) {
        outError = "Sin datos para graficar.";
        return wxBitmap();
    }

    const int width = std::max(requestedSize.GetWidth(), 320);
    const int height = std::max(requestedSize.GetHeight(), 220);
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

    const AxisRange xRange = ComputeAxisRange(xValues);
    const AxisRange yRange = ComputeAxisRange(yValues);

    dc.SetFont(GetFont());
    dc.SetTextForeground(wxColour(194, 202, 215));
    for (int index = 0; index < 5; ++index) {
        const double fraction = static_cast<double>(index) / 4.0;
        const int x = plotRect.GetLeft() + static_cast<int>(std::lround(fraction * plotRect.GetWidth()));
        const int y = plotRect.GetBottom() - static_cast<int>(std::lround(fraction * plotRect.GetHeight()));

        dc.SetPen(wxPen(wxColour(35, 43, 52), 1));
        dc.DrawLine(x, plotRect.GetTop(), x, plotRect.GetBottom());
        dc.DrawLine(plotRect.GetLeft(), y, plotRect.GetRight(), y);

        const double xTick = xRange.minValue + (xRange.maxValue - xRange.minValue) * fraction;
        const double yTick = yRange.minValue + (yRange.maxValue - yRange.minValue) * fraction;
        dc.DrawText(wxString::Format("%.2f", xTick), x - 18, plotRect.GetBottom() + 8);
        dc.DrawText(wxString::Format("%.2f", yTick), 8, y - 7);
    }

    dc.SetTextForeground(wxColour(229, 236, 246));
    dc.DrawText(ToWx(title), plotRect.GetLeft(), 8);
    dc.SetTextForeground(wxColour(146, 200, 255));
    dc.DrawText(ToWx(batview::core::services::PlotService::AxisLabel(xAxis)), plotRect.GetLeft(), height - 24);
    dc.SetTextForeground(wxColour(255, 190, 112));
    dc.DrawText(ToWx(batview::core::services::PlotService::AxisLabel(yAxis)), 8, 8);

    dc.SetPen(wxPen(wxColour(90, 255, 180), 2));
    dc.SetBrush(wxBrush(wxColour(90, 255, 180)));

    std::vector<wxPoint> points;
    points.reserve(measurements.size());
    for (std::size_t index = 0; index < measurements.size(); ++index) {
        const int px = ProjectValue(xValues[index], xRange, plotRect.GetLeft(), plotRect.GetRight());
        const int py = ProjectValue(yValues[index], yRange, plotRect.GetBottom(), plotRect.GetTop());
        points.emplace_back(px, py);
    }

    if (points.size() == 1) {
        dc.DrawCircle(points.front(), 3);
    } else {
        dc.DrawLines(static_cast<int>(points.size()), points.data());
        dc.DrawCircle(points.back(), 3);
    }

    dc.SelectObject(wxNullBitmap);
    return bitmap;
}

void PlotPanel::RefreshPlot(int index) {
    auto* widgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
    if (widgets == nullptr || viewModel_ == nullptr) {
        return;
    }

    const auto measurements = viewModel_->GetMeasurements();
    const auto xAxis = kAxes[static_cast<std::size_t>(widgets->xAxisChoice->GetSelection())];
    const auto yAxis = kAxes[static_cast<std::size_t>(widgets->yAxisChoice->GetSelection())];
    std::string error;
    std::size_t pointCount = 0;
    wxBitmap bitmap = RenderPlotBitmap(measurements,
                                       xAxis,
                                       yAxis,
                                       widgets->bitmap->GetSize(),
                                       widgets->title,
                                       error,
                                       pointCount);

    if (!bitmap.IsOk()) {
        ResetPlotCard(*widgets);
        widgets->status->SetLabel(error.empty() ? "Sin datos para graficar." : ToWx(error));
        return;
    }

    widgets->bitmap->SetBitmap(bitmap);
    widgets->lastPointCount = pointCount;
    widgets->status->SetLabel("Puntos graficados: " + std::to_string(pointCount));

    if (!measurements.empty()) {
        widgets->status->SetLabel("Puntos graficados: "
                                  + std::to_string(pointCount)
                                  + " | "
                                  + FormatPointSummary(measurements.back()));
    }
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
    widgets.bitmap->SetBitmap(wxBitmap(720, 260));
    widgets.lastPointCount = 0;
    widgets.status->SetLabel("Sin datos para graficar.");
}

bool PlotPanel::ExportPlotImage(const std::string& filePath, int index, std::string& outError) const {
    auto* widgets = index == 0 ? primaryPlot_.get() : secondaryPlot_.get();
    if (widgets == nullptr || viewModel_ == nullptr) {
        outError = "No hay grafico disponible para exportar.";
        return false;
    }

    const auto xAxis = kAxes[static_cast<std::size_t>(widgets->xAxisChoice->GetSelection())];
    const auto yAxis = kAxes[static_cast<std::size_t>(widgets->yAxisChoice->GetSelection())];
    std::size_t pointCount = 0;
    wxBitmap bitmap = RenderPlotBitmap(viewModel_->GetMeasurements(),
                                       xAxis,
                                       yAxis,
                                       widgets->bitmap->GetSize(),
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
    RefreshPlots();
}

} // namespace batview::ui::panels

#endif
