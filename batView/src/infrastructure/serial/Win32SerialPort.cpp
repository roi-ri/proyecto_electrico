#include "infrastructure/serial/Win32SerialPort.h"

#ifdef _WIN32

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace batview::infrastructure::serial {

namespace {

HANDLE InvalidSerialHandle() {
    return INVALID_HANDLE_VALUE;
}

DWORD ToBaudRateConstant(int baudRate) {
    switch (baudRate) {
    case 9600:
        return CBR_9600;
    case 19200:
        return CBR_19200;
    case 38400:
        return CBR_38400;
    case 57600:
        return CBR_57600;
    case 115200:
        return CBR_115200;
    default:
        return CBR_115200;
    }
}

std::string TrimAscii(std::string value) {
    auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

} // namespace

Win32SerialPort::~Win32SerialPort() {
    Close();
}

bool Win32SerialPort::Open(const std::string& portName, int baudRate) {
    Close();
    lastError_.clear();
    portName_ = TrimAscii(portName);

    const std::string normalizedPortName = NormalizePortName(portName_);
    HANDLE handle = CreateFileA(normalizedPortName.c_str(),
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                nullptr,
                                OPEN_EXISTING,
                                0,
                                nullptr);
    if (handle == InvalidSerialHandle()) {
        SetLastErrorFromSystem("open", ::GetLastError());
        return false;
    }

    handle_ = handle;
    if (!ConfigurePort(baudRate)) {
        Close();
        return false;
    }

    PurgeComm(static_cast<HANDLE>(handle_), PURGE_RXCLEAR | PURGE_TXCLEAR);
    readBuffer_.clear();
    return true;
}

void Win32SerialPort::Close() {
    if (handle_ != nullptr && static_cast<HANDLE>(handle_) != InvalidSerialHandle()) {
        CloseHandle(static_cast<HANDLE>(handle_));
    }
    handle_ = nullptr;
    portName_.clear();
    readBuffer_.clear();
}

bool Win32SerialPort::IsOpen() const {
    return handle_ != nullptr && static_cast<HANDLE>(handle_) != InvalidSerialHandle();
}

bool Win32SerialPort::WriteLine(const std::string& line) {
    if (!IsOpen()) {
        SetLastError("write failed: serial port is not open.");
        return false;
    }

    std::string payload = line;
    if (payload.empty() || payload.back() != '\n') {
        payload.push_back('\n');
    }

    DWORD bytesWritten = 0;
    if (!WriteFile(static_cast<HANDLE>(handle_),
                   payload.data(),
                   static_cast<DWORD>(payload.size()),
                   &bytesWritten,
                   nullptr)) {
        SetLastErrorFromSystem("write", ::GetLastError());
        return false;
    }

    if (bytesWritten != payload.size()) {
        SetLastError("write failed: serial port accepted a partial frame.");
        return false;
    }

    lastError_.clear();
    return true;
}

bool Win32SerialPort::ReadLine(std::string& outLine, int timeoutMs) {
    if (!IsOpen()) {
        SetLastError("read failed: serial port is not open.");
        return false;
    }

    if (ExtractLineFromBuffer(outLine)) {
        return true;
    }

    if (!ReadAvailableIntoBuffer(timeoutMs)) {
        return false;
    }

    return ExtractLineFromBuffer(outLine);
}

const std::string& Win32SerialPort::GetLastError() const {
    return lastError_;
}

bool Win32SerialPort::ConfigurePort(int baudRate) {
    DCB dcb {};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(static_cast<HANDLE>(handle_), &dcb)) {
        SetLastErrorFromSystem("GetCommState", ::GetLastError());
        return false;
    }

    dcb.BaudRate = ToBaudRateConstant(baudRate);
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;

    if (!SetCommState(static_cast<HANDLE>(handle_), &dcb)) {
        SetLastErrorFromSystem("SetCommState", ::GetLastError());
        return false;
    }

    COMMTIMEOUTS timeouts {};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 250;

    if (!SetCommTimeouts(static_cast<HANDLE>(handle_), &timeouts)) {
        SetLastErrorFromSystem("SetCommTimeouts", ::GetLastError());
        return false;
    }

    SetupComm(static_cast<HANDLE>(handle_), 4096, 4096);
    lastError_.clear();
    return true;
}

bool Win32SerialPort::WaitForCommEvent(unsigned long timeoutMs) const {
    COMSTAT status {};
    DWORD errors = 0;

    const DWORD deadline = GetTickCount() + timeoutMs;
    while (GetTickCount() <= deadline) {
        if (!ClearCommError(static_cast<HANDLE>(handle_), &errors, &status)) {
            return false;
        }

        if (status.cbInQue > 0) {
            return true;
        }

        Sleep(10);
    }

    return false;
}

bool Win32SerialPort::ReadAvailableIntoBuffer(int timeoutMs) {
    if (!IsOpen()) {
        return false;
    }

    if (!WaitForCommEvent(static_cast<DWORD>(std::max(timeoutMs, 0)))) {
        return false;
    }

    char chunk[512];
    DWORD bytesRead = 0;
    if (!ReadFile(static_cast<HANDLE>(handle_), chunk, static_cast<DWORD>(sizeof(chunk)), &bytesRead, nullptr)) {
        SetLastErrorFromSystem("read", ::GetLastError());
        return false;
    }

    if (bytesRead == 0) {
        return false;
    }

    readBuffer_.append(chunk, chunk + bytesRead);
    lastError_.clear();
    return true;
}

bool Win32SerialPort::ExtractLineFromBuffer(std::string& outLine) {
    const std::size_t lineEnd = readBuffer_.find('\n');
    if (lineEnd == std::string::npos) {
        return false;
    }

    outLine = readBuffer_.substr(0, lineEnd);
    if (!outLine.empty() && outLine.back() == '\r') {
        outLine.pop_back();
    }

    readBuffer_.erase(0, lineEnd + 1);
    return true;
}

void Win32SerialPort::SetLastError(const std::string& message) {
    lastError_ = message;
}

void Win32SerialPort::SetLastErrorFromSystem(const std::string& operation, unsigned long errorCode) {
    std::vector<char> buffer(512, '\0');
    const DWORD result = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                        nullptr,
                                        errorCode,
                                        0,
                                        buffer.data(),
                                        static_cast<DWORD>(buffer.size()),
                                        nullptr);

    std::string message = operation + " failed";
    if (!portName_.empty()) {
        message += " on " + portName_;
    }
    message += ": ";
    message += result > 0 ? TrimAscii(buffer.data()) : "unknown system error";
    message += " (code=" + std::to_string(errorCode) + ")";
    lastError_ = std::move(message);
}

std::string Win32SerialPort::NormalizePortName(const std::string& portName) const {
    if (portName.rfind("\\\\.\\", 0) == 0) {
        return portName;
    }

    if (portName.size() > 3 && (portName.rfind("COM", 0) == 0 || portName.rfind("com", 0) == 0)) {
        return "\\\\.\\" + portName;
    }

    return portName;
}

} // namespace batview::infrastructure::serial

#endif
