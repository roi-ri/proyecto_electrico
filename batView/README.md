# batView

Aplicación de escritorio en C++ para controlar pruebas de baterías sobre un ESP32 vía USB/UART.

El proyecto combina wxWidgets para la interfaz, una capa de servicios para la lógica de protocolo y adaptadores de infraestructura para serial, logging y exportación.

## Resumen

batView está orientado a ejecutar y supervisar pruebas de baterías desde una interfaz de escritorio, con telemetría recibida en tiempo real desde el ESP32 y exportación de los resultados en formatos útiles para análisis y trazabilidad.

## Características principales

- Conexión serial real a ESP32 en macOS/Linux mediante backend POSIX.
- Handshake de conexión con reintento automático para boards que reinician al abrir el puerto.
- Flujo operativo guiado para conectar, seleccionar batería, elegir función, iniciar carga, descarga o ciclado y detener con `#STOP`.
- Recepción de telemetría `#DATA`.
- Registro de tráfico ESP32 <-> PC en la UI.
- Exportación de mediciones a CSV.
- Exportación de mediciones a MAT y XLSX.
- Exportación de gráficas a CSV, MAT, XLSX y PNG.
- Panel de gráficas con renderizado real y resumen del último punto capturado.
- Sketches de prueba para Arduino IDE y PlatformIO.

## Estructura del proyecto

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

## Arquitectura del código

- `src/app/`: punto de entrada y composición de la aplicación.
- `src/core/`: protocolo y lógica de negocio.
- `src/infrastructure/`: adaptadores para CSV, logging y serial.
- `src/models/`: entidades y estructuras de dominio.
- `src/ui/`: ventanas, paneles, diálogos y viewmodels.

## Protocolo soportado

### Comandos enviados por la aplicación

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

- `voltaje`: valor enviado por el ESP32.
- `corriente`: valor enviado por el ESP32.
- `timestamp`: valor enviado por el ESP32 en milisegundos.
- `estado`: valor enviado por el ESP32 cuando se usa la variante larga.
- `ciclos`: valor enviado por el ESP32 cuando se usa la variante larga.

La aplicación no fabrica esos valores. Lo único que hace en la gráfica es convertir `timestamp` de milisegundos a segundos para mostrar un eje de tiempo más legible. El dato original almacenado sigue siendo el que envía el ESP32.

## Build y ejecución multiplataforma

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

##### Notas para Ubuntu

Si clonas o copias el repo desde otro sistema operativo, no reutilices el mismo `build-release` entre máquinas (por ejemplo macOS -> Ubuntu), porque CMake guarda rutas absolutas en cache.

Instala dependencias (Ubuntu 24.04):

```bash
sudo apt update
sudo apt install -y cmake g++ make pkg-config libwxgtk3.2-dev libgtk-3-dev
```

Si `find_package(wxWidgets)` falla, también puedes instalar:

```bash
sudo apt install -y wx3.2-headers wx-common
```

Reconfigura desde cero en Ubuntu:

```bash
rm -rf build-release
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j"$(nproc)"
./build-release/batView
```

Errores comunes:

- `CMakeCache.txt directory is different`: elimina `build-release` y vuelve a configurar.
- `Could NOT find wxWidgets`: instala los paquetes de desarrollo de wxWidgets (`libwxgtk3.2-dev`).

#### Windows

```powershell
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON
cmake --build build-release --config Release
.\build-release\Release\batView.exe
```

### Empaquetado

```bash
cmake --build build-release --config Release --target package
```

### Publicación automática (GitHub Releases)

El repositorio incluye dos workflows de GitHub Actions:

- `.github/workflows/package.yml`: build y empaquetado en macOS, Ubuntu y Windows con artefactos descargables.
- `.github/workflows/release.yml`: genera y publica una Release con artefactos cuando se empuja un tag de versión.

Para publicar instaladores/paquetes en una Release:

```bash
git tag v1.0.1
git push origin v1.0.1
```

Después del push del tag, GitHub Actions construye en los 3 sistemas operativos y adjunta los paquetes generados a la Release del tag.

### Suite de tests

```bash
cmake -S . -B build-tests -DBATVIEW_BUILD_TESTS=ON
cmake --build build-tests --config Release
ctest --test-dir build-tests --output-on-failure
```

## Artefactos

- El paquete generado queda fuera del directorio de build, en `dist/`.

## Ejemplos para ESP32

- [examples/arduino/esp32_batview_test](./examples/arduino/esp32_batview_test/README.md)
- [examples/platformio/esp32_batview_test](./examples/platformio/esp32_batview_test/README.md)

## Documentación

- [docs/PROJECT_DOCUMENTATION.md](./docs/PROJECT_DOCUMENTATION.md)
- [docs/ARCHITECTURE.md](./docs/ARCHITECTURE.md)
- [docs/DOWNLOAD_AND_INSTALL.md](./docs/DOWNLOAD_AND_INSTALL.md)

## Estado del proyecto

El proyecto ya cuenta con una base funcional sólida para pruebas reales con ESP32.

Mejoras ya incorporadas:

- Gráficas reales renderizadas desde la app.
- Mayor cobertura de pruebas para exportación y plots.
- Exportación de sesión y de gráficos a MAT/XLSX.
- Empaquetado distribuible en `dist/` con artefactos por plataforma.

Áreas que todavía pueden crecer:

- Firma/certificación de instaladores por plataforma.
- Detección y empaquetado automático del runtime Python embebido para XLSX en distribuciones finales.
- Más pruebas de integración con hardware real.
