#pragma once

#include <string>

#include "ui/viewmodels/MainViewModel.h"

namespace batview::app {

class AppController {
public:
    explicit AppController(ui::viewmodels::MainViewModel& mainViewModel);

    void Connect(const std::string& portName);
    bool SelectBatteryForFunction(int batteryTypeCode, int functionCode);
    void StartCycle(bool indefiniteMode, int cycleCount);
    void StopCycle();
    bool StartLoad(int targetPercent);
    bool StartUnload(int targetPercent);

private:
    ui::viewmodels::MainViewModel& mainViewModel_;
};

} // namespace batview::app
