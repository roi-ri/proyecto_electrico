#include "infrastructure/serial/PosixSerialPort.h"

#include <cerrno>
#include <cstring>

#include <fcntl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

namespace batview::infrastructure::serial {

namespace {

speed_t ToBaudRateConstant(int baudRate) {
    switch (baudRate) {
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    default:
        return B115200;
    }
}

} // namespace

PosixSerialPort::~PosixSerialPort() {
    Close();
}

bool PosixSerialPort::Open(const std::string& portName, int baudRate) {
    Close();
    lastError_.clear();
    portName_ = portName;

    fd_ = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd_ < 0) {
        SetLastErrorFromErrno("open", portName);
        return false;
    }

    if (!ConfigurePort(baudRate)) {
        Close();
        return false;
    }

    readBuffer_.clear();
    return true;
}

void PosixSerialPort::Close() {
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
    portName_.clear();
    readBuffer_.clear();
}

bool PosixSerialPort::IsOpen() const {
    return fd_ >= 0;
}

bool PosixSerialPort::WriteLine(const std::string& line) {
    if (!IsOpen()) {
        SetLastError("write failed: serial port is not open.");
        return false;
    }

    std::string payload = line;
    if (payload.empty() || payload.back() != '\n') {
        payload.push_back('\n');
    }

    std::size_t totalWritten = 0;
    while (totalWritten < payload.size()) {
        const ssize_t written = write(fd_, payload.data() + totalWritten, payload.size() - totalWritten);
        if (written > 0) {
            totalWritten += static_cast<std::size_t>(written);
            continue;
        }

        if (written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            fd_set writeSet;
            FD_ZERO(&writeSet);
            FD_SET(fd_, &writeSet);
            timeval timeout {0, 200000};
            const int result = select(fd_ + 1, nullptr, &writeSet, nullptr, &timeout);
            if (result > 0) {
                continue;
            }
            if (result < 0) {
                SetLastErrorFromErrno("select while waiting to write", portName_);
                return false;
            }
            SetLastError("write timeout while waiting for serial port to become writable on " + portName_);
            return false;
        }

        SetLastErrorFromErrno("write", portName_);
        return false;
    }

    lastError_.clear();
    return true;
}

bool PosixSerialPort::ReadLine(std::string& outLine, int timeoutMs) {
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

bool PosixSerialPort::ConfigurePort(int baudRate) {
    termios tty {};
    if (tcgetattr(fd_, &tty) != 0) {
        SetLastErrorFromErrno("tcgetattr", portName_);
        return false;
    }

    cfmakeraw(&tty);
    const speed_t speed = ToBaudRateConstant(baudRate);
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
        SetLastErrorFromErrno("tcsetattr", portName_);
        return false;
    }

    lastError_.clear();
    return true;
}

bool PosixSerialPort::ReadAvailableIntoBuffer(int timeoutMs) {
    if (!IsOpen()) {
        return false;
    }

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(fd_, &readSet);

    timeval timeout {};
    timeout.tv_sec = timeoutMs / 1000;
    timeout.tv_usec = static_cast<suseconds_t>((timeoutMs % 1000) * 1000);

    const int selectResult = select(fd_ + 1, &readSet, nullptr, nullptr, &timeout);
    if (selectResult <= 0) {
        if (selectResult < 0) {
            SetLastErrorFromErrno("select while waiting to read", portName_);
        }
        return false;
    }

    char chunk[512];
    const ssize_t bytesRead = read(fd_, chunk, sizeof(chunk));
    if (bytesRead <= 0) {
        if (bytesRead < 0) {
            SetLastErrorFromErrno("read", portName_);
        }
        return false;
    }

    readBuffer_.append(chunk, static_cast<std::size_t>(bytesRead));
    lastError_.clear();
    return true;
}

bool PosixSerialPort::ExtractLineFromBuffer(std::string& outLine) {
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

const std::string& PosixSerialPort::GetLastError() const {
    return lastError_;
}

void PosixSerialPort::SetLastError(const std::string& message) {
    lastError_ = message;
}

void PosixSerialPort::SetLastErrorFromErrno(const std::string& operation, const std::string& portName) {
    std::string message = operation + " failed";
    if (!portName.empty()) {
        message += " on " + portName;
    }
    message += ": ";
    message += std::strerror(errno);
    message += " (errno=" + std::to_string(errno) + ")";
    lastError_ = std::move(message);
}

} // namespace batview::infrastructure::serial
