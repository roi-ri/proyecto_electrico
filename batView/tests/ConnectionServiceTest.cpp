#include <gtest/gtest.h>

#include <queue>
#include <string>

#include "core/services/ConnectionService.h"

namespace batview::core::services {

namespace {

class TestSerialPort : public ISerialPort {
public:
    bool Open(const std::string& portName, int baudRate) override {
        lastPortName = portName;
        lastBaudRate = baudRate;
        isOpen = portName == "/dev/ttyUSB0";
        return isOpen;
    }

    void Close() override {
        isOpen = false;
    }

    bool IsOpen() const override {
        return isOpen;
    }

    bool WriteLine(const std::string& line) override {
        lastWrittenLine = line;
        ++writeCount;
        return isOpen;
    }

    bool ReadLine(std::string& outLine, int timeoutMs) override {
        (void)timeoutMs;

        if (!isOpen || incomingFrames.empty()) {
            return false;
        }

        outLine = incomingFrames.front();
        incomingFrames.pop();
        return true;
    }

    const std::string& GetLastError() const override {
        return lastError;
    }

    void QueueIncomingFrame(const std::string& frame) {
        incomingFrames.push(frame);
    }

    std::queue<std::string> incomingFrames;
    std::string lastPortName;
    int lastBaudRate = 0;
    std::string lastWrittenLine;
    int writeCount = 0;
    bool isOpen = false;
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

TEST(ConnectionServiceTest, ConnectSuccess) {
    TestSerialPort serialPort;
    TestLogger logger;
    ConnectionService connectionService(serialPort, logger);

    serialPort.QueueIncomingFrame("#ACK,CONECTION");

    EXPECT_TRUE(connectionService.Connect("/dev/ttyUSB0", 115200));
    EXPECT_TRUE(serialPort.IsOpen());
    EXPECT_EQ(serialPort.lastPortName, "/dev/ttyUSB0");
    EXPECT_EQ(serialPort.lastBaudRate, 115200);
    EXPECT_EQ(serialPort.lastWrittenLine, "#CONNECTION");
    EXPECT_GE(serialPort.writeCount, 1);
    EXPECT_EQ(logger.lastInfo, "Serial connection established on /dev/ttyUSB0");
    EXPECT_TRUE(connectionService.GetLastError().empty());
}

TEST(ConnectionServiceTest, RetriesHandshakeAfterReadyStatus) {
    TestSerialPort serialPort;
    TestLogger logger;
    ConnectionService connectionService(serialPort, logger);

    serialPort.QueueIncomingFrame("#STATUS,BOOT,ESP32_READY");
    serialPort.QueueIncomingFrame("#ACK,CONNECTION");

    EXPECT_TRUE(connectionService.Connect("/dev/ttyUSB0", 115200));
    EXPECT_TRUE(serialPort.IsOpen());
    EXPECT_GE(serialPort.writeCount, 2);
    EXPECT_TRUE(connectionService.GetLastError().empty());
}

TEST(ConnectionServiceTest, ConnectFailure) {
    TestSerialPort serialPort;
    TestLogger logger;
    ConnectionService connectionService(serialPort, logger);

    EXPECT_FALSE(connectionService.Connect("/dev/ttyUSB1", 115200));
    EXPECT_FALSE(serialPort.IsOpen());
    EXPECT_EQ(logger.lastError, "No se pudo abrir el puerto serial: /dev/ttyUSB1");
    EXPECT_EQ(connectionService.GetLastError(), "No se pudo abrir el puerto serial: /dev/ttyUSB1");
}

TEST(ConnectionServiceTest, HandshakeTimeout) {
    TestSerialPort serialPort;
    TestLogger logger;
    ConnectionService connectionService(serialPort, logger);

    EXPECT_FALSE(connectionService.Connect("/dev/ttyUSB0", 115200));
    EXPECT_FALSE(serialPort.IsOpen());
    EXPECT_EQ(serialPort.lastWrittenLine, "#CONNECTION");
    EXPECT_EQ(logger.lastError, "El puerto se abrio, pero el ESP32 no envio #ACK,CONNECTION dentro de 6 segundos.");
    EXPECT_EQ(connectionService.GetLastError(),
              "El puerto se abrio, pero el ESP32 no envio #ACK,CONNECTION dentro de 6 segundos.");
}

} // namespace batview::core::services
