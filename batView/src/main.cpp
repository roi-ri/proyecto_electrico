#include <wx/wx.h>
#include "ui/frames/MainFrame.h"

class BatViewApp : public wxApp {
public:
    bool OnInit() override {
        auto* frame = new MainFrame("batView");
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(BatViewApp);
