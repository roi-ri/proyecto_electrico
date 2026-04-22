#pragma once

#include <string>

namespace batview::core::services {

/**
 * @brief Abstracción del transporte serial.
 *
 * Esta interfaz desacopla la lógica de aplicación de la implementación
 * concreta del puerto serial para cada plataforma.
 */
class ISerialPort {
public:
    virtual ~ISerialPort() = default;

    virtual bool Open(const std::string& portName, int baudRate) = 0;
    virtual void Close() = 0;
    virtual bool IsOpen() const = 0;

    virtual bool WriteLine(const std::string& line) = 0;
    virtual bool ReadLine(std::string& outLine, int timeoutMs) = 0;
};

} // namespace batview::core::services
