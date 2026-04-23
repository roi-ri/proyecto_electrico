#include "app/AppController.h"

namespace batview::app {

AppController::AppController(ui::viewmodels::MainViewModel& mainViewModel)
    : mainViewModel_(mainViewModel) {}

void AppController::Connect(const std::string& portName) {
    mainViewModel_.ConnectToDevice(portName);
}

bool AppController::SelectBatteryForFunction(int batteryTypeCode, int functionCode) {
    return mainViewModel_.SendBatterySelection(batteryTypeCode, functionCode);
}

void AppController::StartCycle(bool indefiniteMode, int cycleCount) {
    if (!mainViewModel_.SendCycleConfiguration(indefiniteMode, cycleCount)) {
        return;
    }

    mainViewModel_.StartAcquisition();
}

void AppController::StopCycle() {
    mainViewModel_.SendStopCommand();
    mainViewModel_.StopAcquisition();
}

bool AppController::StartLoad(int targetPercent) {
    if (!mainViewModel_.SendLoadTarget(targetPercent)) {
        return false;
    }

    mainViewModel_.StartAcquisition();
    return true;
}

bool AppController::StartUnload(int targetPercent) {
    if (!mainViewModel_.SendUnloadTarget(targetPercent)) {
        return false;
    }

    mainViewModel_.StartAcquisition();
    return true;
}

} // namespace batview::app
