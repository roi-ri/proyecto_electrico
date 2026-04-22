#include "ui/panels/PlotPanel.h"
#include <wx/dc.h>
#include <wx/paint.h>

namespace batview::ui::panels {

PlotPanel::PlotPanel(wxWindow* parent)
    : wxPanel(parent) {
    Bind(wxEVT_PAINT, &PlotPanel::OnPaint, this);
}

void PlotPanel::SetMeasurements(const std::vector<batview::core::models::Measurement>& measurements) {
    measurements_ = measurements;
    Refresh();  // Redibujar el panel cuando se actualicen los datos
}

void PlotPanel::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    DrawGraph(dc);
}

void PlotPanel::DrawGraph(wxDC& dc) {
    if (measurements_.empty()) {
        return;
    }

    // Establecemos el tamaño de las gráficas
    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(50, 20, 600, 300);  // Dibuja el área para las gráficas

    // Graficar voltaje y corriente (simplificación con líneas)
    dc.SetPen(*wxBLUE_PEN);  // Voltaje en azul
    for (size_t i = 1; i < measurements_.size(); ++i) {
        dc.DrawLine(50 + (i - 1) * 10, 320 - measurements_[i - 1].voltage * 20, 
                    50 + i * 10, 320 - measurements_[i].voltage * 20);
    }

    dc.SetPen(*wxRED_PEN);  // Corriente en rojo
    for (size_t i = 1; i < measurements_.size(); ++i) {
        dc.DrawLine(50 + (i - 1) * 10, 320 - measurements_[i - 1].current * 20, 
                    50 + i * 10, 320 - measurements_[i].current * 20);
    }
}

} // namespace batview::ui::panels
