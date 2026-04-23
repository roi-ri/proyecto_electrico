#pragma once

#ifdef _WIN32
#include "infrastructure/serial/Win32SerialPort.h"
#else
#include "infrastructure/serial/PosixSerialPort.h"
#endif

namespace batview::infrastructure::serial {

#ifdef _WIN32
using NativeSerialPort = Win32SerialPort;
#else
using NativeSerialPort = PosixSerialPort;
#endif

} // namespace batview::infrastructure::serial
