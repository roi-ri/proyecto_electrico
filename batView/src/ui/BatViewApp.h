#pragma once

#ifdef BATVIEW_ENABLE_WX

#include <wx/app.h>

namespace batview::ui {

class BatViewApp : public wxApp {
public:
    bool OnInit() override;
};

} // namespace batview::ui

#endif
