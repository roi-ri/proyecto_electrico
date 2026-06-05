#include "core/services/ConnectionService.h"

#include <algorithm>
#include <cctype>
#include <chrono>

namespace batview::core::services {

namespace {

constexpr auto kConnectionTimeout = std::chrono::milliseconds(6000);
constexpr auto kHandshakeRetry = std::chrono::milliseconds(1200);

std::string ToUpperAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return value;
}

bool IsConnectionAck(const std::string& line) {
    if (line.rfind("#ACK,", 0) != 0) {
        return false;
    }

    const auto token = ToUpperAscii(line.substr(5));
    return token.find("CONNECTION") != std::string::npos || token.find("CONECTION") != std::string::npos;
}

bool IsReadyStatus(const std::string& line) {
    if (line.rfind("#STATUS,", 0) != 0) {
        return false;
    }

    const auto token = ToUpperAscii(line);
    return token.find("ESP32_READY") != std::string::npos || token.find("CONNECTED") != std::string::npos;
}

} // namespace

ConnectionService::ConnectionService(ISerialPort& serialPort, ILogger& logger)
    : serialPort_(serialPort), logger_(logger) {}

bool ConnectionService::Connect(const std::string& portName, int baudRate) {
    lastError_.clear();

    if (!serialPort_.Open(portName, baudRate)) {
        lastError_ = "No se pudo abrir el puerto serial: " + portName;
        if (!serialPort_.GetLastError().empty()) {
            lastError_ += " | detalle: " + serialPort_.GetLastError();
        }
        logger_.Error(lastError_);
        return false;
    }

    const std::string handshakeCommand = "#CONNECTION";
    const auto started = std::chrono::steady_clock::now();
    const auto timeout = kConnectionTimeout;
    auto lastHandshakeSentAt = started - kHandshakeRetry;
    bool readyObserved = false;

    auto sendHandshake = [&]() -> bool {
        if (!serialPort_.WriteLine(handshakeCommand)) {
            lastError_ = "Se abrio el puerto, pero no se pudo enviar #CONNECTION.";
            if (!serialPort_.GetLastError().empty()) {
                lastError_ += " | detalle: " + serialPort_.GetLastError();
            }
            logger_.Error(lastError_);
            serialPort_.Close();
            return false;
        }

        lastHandshakeSentAt = std::chrono::steady_clock::now();

        if (trafficObserver_) {
            trafficObserver_(true, handshakeCommand);
        }
        return true;
    };

    if (!sendHandshake()) {
        return false;
    }

    if (trafficObserver_) {
        trafficObserver_(true, "Waiting up to 45 seconds for ACK_connection...");
    }

    while (std::chrono::steady_clock::now() - started < timeout) {
        const auto now = std::chrono::steady_clock::now();
        if (now - lastHandshakeSentAt >= kHandshakeRetry) {
            if (!sendHandshake()) {
                return false;
            }
        }

        std::string line;
        if (!serialPort_.ReadLine(line, 100)) {
            continue;
        }

        if (trafficObserver_) {
            trafficObserver_(false, line);
        }

        if (IsConnectionAck(line)) {
            lastError_.clear();
            logger_.Info("Serial connection established on " + portName);
            return true;
        }

        if (IsReadyStatus(line) && !readyObserved) {
            readyObserved = true;
            logger_.Info("ESP32 reported ready state while waiting for ACK_connection; retrying handshake.");
            if (!sendHandshake()) {
                return false;
            }
        }
    }

    lastError_ = "El puerto se abrio, pero el ESP32 no envio #ACK,CONNECTION dentro de " +
                 std::to_string(timeout.count() / 1000) + " segundos.";
    if (!serialPort_.GetLastError().empty()) {
        lastError_ += " | ultimo detalle serial: " + serialPort_.GetLastError();
    }
    logger_.Error(lastError_);
    if (trafficObserver_) {
        trafficObserver_(true, lastError_);
    }
    serialPort_.Close();
    return false;
}

void ConnectionService::Disconnect() {
    serialPort_.Close();
    logger_.Info("Serial connection closed.");
}

void ConnectionService::SetTrafficObserver(TrafficObserver observer) {
    trafficObserver_ = std::move(observer);
}

const std::string& ConnectionService::GetLastError() const {
    return lastError_;
}

} // namespace batview::core::services
