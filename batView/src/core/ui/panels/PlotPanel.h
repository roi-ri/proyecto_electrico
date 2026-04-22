#pragma once

#include <wx/panel.h>
#include <wx/dcclient.h>
#include <vector>
#include "core/models/Measurement.h"

namespace batview::ui::panels {

/**
 * @brief Panel de visualización de gráficas de datos adquiridos.
 */
class PlotPanel : public wxPanel {
public:
    explicit PlotPanel(wxWindow* parent);

    void SetMeasurements(const std::vector<batview::core::models::Measurement>& measurements);

private:
    void OnPaint(wxPaintEvent& event);
    void DrawGraph(wxDC& dc);

    std::vector<batview::core::models::Measurement> measurements_;
};

} // namespace batview::ui::panels
