#pragma once

#include "ISerialPort.h"

namespace batview::infrastructure::serial {

/**
 * @brief Implementación concreta del puerto serial para macOS.
 */
class MacSerialPort : public ISerialPort {
public:
    bool Open(const std::string& portName, int baudRate) override;
    void Close() override;
    bool IsOpen() const override;
    bool WriteLine(const std::string& line) override;
    bool ReadLine(std::string& outLine, int timeoutMs) override;

private:
    // Aquí se deben gestionar detalles internos, como la configuración del puerto
    // para macOS utilizando bibliotecas de serialización como termios o similar
};

} // namespace batview::infrastructure::serial
