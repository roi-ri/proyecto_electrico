# batView

Aplicación de escritorio en C++ para controlar pruebas de baterías sobre un ESP32 vía USB/UART.  
El proyecto usa wxWidgets para la interfaz, una capa de servicios para la lógica de protocolo y adaptadores de infraestructura para serial, logging y exportación.

## Capacidades actuales

- Conexión serial real a ESP32 en macOS/Linux mediante backend POSIX.
- Handshake de conexión con reintento automático para boards que reinician al abrir el puerto.
- Flujo operativo guiado:
  - conectar
  - seleccionar batería
  - seleccionar función
  - iniciar carga, descarga o ciclado
  - detener con `#STOP`
- Recepción de telemetría `#DATA`.
- Registro de tráfico `ESP32 <-> PC` en la UI.
- Exportación de mediciones a CSV.
- Sketchs de prueba para Arduino IDE y PlatformIO.

## Estructura

```text
src/
  app/
  core/
    protocol/
    services/
  infrastructure/
    csv/
    logging/
    serial/
  models/
  ui/
    dialogs/
    frames/
    panels/
    viewmodels/
docs/
examples/
tests/
```

## Protocolo soportado

### Comandos enviados por la app

- `#CONNECTION`
- `#DATA,1,<tipoBateria>,<funcion>`
- `#CICLE,0,0`
- `#CICLE,1,<numeroCiclos>`
- `#LOAD,<0-100>`
- `#UNLOAD,<0-100>`
- `#STOP`

### Respuestas esperadas del ESP32

- `#ACK,CONNECTION` o `#ACK,CONECTION`
- `#ACK,DATA`
- `#ACK,CICLE`
- `#ACK,LOAD`
- `#ACK,UNLOAD`
- `#ACK,STOP`
- `#STATUS,<modo>,<detalle>`
- `#ERROR,<codigo>,<mensaje>`
- `#DATA,<voltaje>,<corriente>,<timestamp>`
- `#DATA,<voltaje>,<corriente>,<timestamp>,<estado>,<ciclos>`

### Origen de los datos de telemetría

Los datos que la aplicación grafica y exporta provienen del ESP32 a través de las tramas `#DATA`.

- `voltaje`: valor enviado por el ESP32
- `corriente`: valor enviado por el ESP32
- `timestamp`: valor enviado por el ESP32 en milisegundos
- `estado`: valor enviado por el ESP32 cuando se usa la variante larga
- `ciclos`: valor enviado por el ESP32 cuando se usa la variante larga

La aplicación no fabrica esos valores.  
Lo único que hace en la gráfica es convertir `timestamp` de milisegundos a segundos para mostrar un eje de tiempo más legible. El dato original almacenado sigue siendo el que envía el ESP32.

## Build y ejecucion multiplataforma

### App wxWidgets

#### macOS

```bash
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON
cmake --build build-release --config Release
open build-release/batView.app
```

#### Linux

```bash
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON
cmake --build build-release --config Release
./build-release/batView
```

#### Windows

```powershell
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON
cmake --build build-release --config Release
.\build-release\Release\batView.exe
```

### Paquete listo para compartir

```bash
cmake --build build-release --config Release --target package
```

El paquete generado queda fuera del directorio de build, en `dist/`.

### Suite de tests

```bash
cmake -S . -B build-tests -DBATVIEW_BUILD_TESTS=ON
cmake --build build-tests --config Release
ctest --test-dir build-tests --output-on-failure
```

## Ejemplos para ESP32

- [examples/arduino/esp32_batview_test](./examples/arduino/esp32_batview_test/README.md)
- [examples/platformio/esp32_batview_test](./examples/platformio/esp32_batview_test/README.md)

## Documentación

- [docs/PROJECT_DOCUMENTATION.md](./docs/PROJECT_DOCUMENTATION.md)
- [docs/ARCHITECTURE.md](./docs/ARCHITECTURE.md)
- [docs/DOWNLOAD_AND_INSTALL.md](./docs/DOWNLOAD_AND_INSTALL.md)

## Estado del proyecto

El proyecto ya cuenta con una base funcional para pruebas reales con ESP32, pero sigue siendo una plataforma de evolución. Las áreas más claras de crecimiento son:

- gráficas reales
- más cobertura de pruebas
- exportación MAT/XLSX
- empaquetado instalable más pulido por plataforma
