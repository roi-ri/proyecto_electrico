#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <queue>
#include <string>
#include <thread>

#include "core/protocol/TextMessageParser.h"
#include "core/services/AcquisitionService.h"

namespace batview::core::services {

namespace {

class TestSerialPort : public ISerialPort {
public:
    bool Open(const std::string& portName, int baudRate) override {
        (void)portName;
        (void)baudRate;
        isOpen = true;
        return true;
    }

    void Close() override {
        isOpen = false;
    }

    bool IsOpen() const override {
        return isOpen;
    }

    bool WriteLine(const std::string& line) override {
        if (!isOpen) {
            return false;
        }

        pendingLines.push(line);
        return true;
    }

    bool ReadLine(std::string& outLine, int timeoutMs) override {
        (void)timeoutMs;
        if (!isOpen || pendingLines.empty()) {
            return false;
        }

        outLine = pendingLines.front();
        pendingLines.pop();
        return true;
    }

    const std::string& GetLastError() const override {
        return lastError;
    }

    bool isOpen = false;
    std::queue<std::string> pendingLines;
    std::string lastError;
};

class TestLogger : public ILogger {
public:
    void Info(const std::string& message) override {
        lastInfo = message;
    }

    void Warning(const std::string& message) override {
        lastWarning = message;
    }

    void Error(const std::string& message) override {
        lastError = message;
    }

    std::string lastInfo;
    std::string lastWarning;
    std::string lastError;
};

} // namespace

TEST(AcquisitionServiceTest, CapturesTelemetryFrames) {
    TestSerialPort serialPort;
    TestLogger logger;
    serialPort.Open("/dev/ttyUSB0", 115200);
    serialPort.pendingLines.push("#DATA,4.12,0.85,1520,1,3");
    serialPort.pendingLines.push("#STATUS,CYCLING,PROCESS_ACTIVE");

    auto parser = std::make_unique<batview::core::protocol::TextMessageParser>();
    AcquisitionService acquisitionService(serialPort, std::move(parser), logger);

    acquisitionService.StartAcquisition();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    acquisitionService.StopAcquisition();

    const auto measurements = acquisitionService.GetMeasurements();
    ASSERT_EQ(measurements.size(), 1u);
    EXPECT_DOUBLE_EQ(measurements.front().voltage, 4.12);
    EXPECT_DOUBLE_EQ(measurements.front().current, 0.85);
    EXPECT_EQ(measurements.front().timestampMs, 1520u);
    EXPECT_EQ(logger.lastWarning, std::string());
}

TEST(AcquisitionServiceTest, StopsCleanly) {
    TestSerialPort serialPort;
    TestLogger logger;
    serialPort.Open("/dev/ttyUSB0", 115200);

    auto parser = std::make_unique<batview::core::protocol::TextMessageParser>();
    AcquisitionService acquisitionService(serialPort, std::move(parser), logger);

    acquisitionService.StartAcquisition();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    acquisitionService.StopAcquisition();

    EXPECT_FALSE(acquisitionService.IsAcquiring());
}

} // namespace batview::core::services
