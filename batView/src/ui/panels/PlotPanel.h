#pragma once

#ifdef BATVIEW_ENABLE_WX

#include <wx/panel.h>
#include <wx/timer.h>

#include <functional>
#include <memory>
#include <string>

#include "core/services/PlotService.h"
#include "ui/viewmodels/MainViewModel.h"

namespace batview::ui::panels {

class PlotPanel : public wxPanel {
public:
    explicit PlotPanel(wxWindow* parent);

    void SetViewModel(std::shared_ptr<batview::ui::viewmodels::MainViewModel> viewModel);

private:
    struct PlotWidgets;

    wxBitmap RenderPlotBitmap(const std::vector<batview::core::models::Measurement>& measurements,
                              batview::core::services::PlotAxis xAxis,
                              batview::core::services::PlotAxis yAxis,
                              const wxSize& size,
                              const std::string& title,
                              std::string& outError,
                              std::size_t& outPointCount) const;
    void BuildPlotCard(wxWindow* parent,
                       wxSizer* parentSizer,
                       const std::string& title,
                       int index);
    void RefreshPlots();
    void RefreshPlot(int index);
    void ExportPlot(int index);
    bool ExportPlotImage(const std::string& filePath, int index, std::string& outError) const;
    void ClearPlots();
    void ResetPlotCard(PlotWidgets& widgets);
    void OnRefreshTimer(wxTimerEvent& event);

    std::shared_ptr<batview::ui::viewmodels::MainViewModel> viewModel_;
    std::unique_ptr<PlotWidgets> primaryPlot_;
    std::unique_ptr<PlotWidgets> secondaryPlot_;
    wxButton* clearPlotsButton_;
    wxTimer refreshTimer_;
};

} // namespace batview::ui::panels

#endif
