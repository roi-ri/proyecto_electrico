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
batView/
│
├── README.md                         guia principal del proyecto
├── CMakeLists.txt                    configuracion de build, empaquetado y tests
├── build_app.sh                      build full en macOS/Linux
├── build_app.ps1                     build full en Windows
├── install_macos.sh                  prepara herramientas y empaqueta en macOS
├── install_linux.sh                  prepara herramientas y empaqueta en Linux
├── install_windows.ps1               prepara herramientas y empaqueta en Windows
│
├── src/                              codigo fuente de la app
│   ├── main.cpp                      entrada para modo consola/stub
│   ├── app/                          composicion de servicios de aplicacion
│   │   ├── AppController.h
│   │   └── AppController.cpp
│   ├── core/                         protocolo y logica de negocio
│   │   ├── protocol/                 codec, parser y tipos de mensajes seriales
│   │   └── services/                 conexion, adquisicion, exportacion y sesiones
│   ├── infrastructure/               adaptadores concretos
│   │   ├── csv/                      exportacion CSV
│   │   ├── logging/                  logger de consola
│   │   ├── python/                   motor embebido para graficas/exportacion
│   │   └── serial/                   puertos seriales POSIX, Windows y stub
│   ├── models/                       estructuras de mediciones
│   └── ui/                           interfaz wxWidgets
│       ├── BatViewApp.h/.cpp         arranque grafico de la aplicacion
│       ├── dialogs/                  dialogos como exportacion
│       ├── frames/                   ventana principal y flujo guiado
│       ├── panels/                   paneles de conexion y graficas
│       └── viewmodels/               puente entre UI y servicios
│
├── assets/                           iconos e imagenes de la app
│   ├── BatView.png
│   └── batView.gif
│
├── docs/                             documentacion de uso, arquitectura e instalacion
│   ├── ARCHITECTURE.md               decisiones de arquitectura
│   ├── BUILD_INSTALLER.md            scripts por sistema operativo
│   └── PROJECT_DOCUMENTATION.md      documentacion tecnica general
│
├── examples/                         firmware de prueba para ESP32
│   ├── arduino/                      sketch para Arduino IDE
│   └── platformio/                   proyecto PlatformIO
│
├── tests/                            pruebas unitarias de servicios
│   ├── AcquisitionServiceTest.cpp
│   ├── ConnectionServiceTest.cpp
│   ├── ExportServiceTest.cpp
│   └── PlotServiceTest.cpp
│
├── scripts/                          utilidades de build
│   └── build_app.py                  prepara runtime Python, compila y empaqueta
│
├── cmake/                            plantillas de empaquetado nativo
├── python/                           runtime Python embebido generado localmente
├── build-release/                    salida local de compilacion
├── dist/                             instaladores/paquetes generados
├── data/                             datos de sesion o salidas locales
└── logs/                             registros locales
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
- `#Battery,<BatteryNameID>,<V@max>,<V@min>,<Amax>`
- `#CICLE,0,0`
- `#CICLE,1,<numeroCiclos>`
- `#LOAD,<0-100>`
- `#UNLOAD,<0-100>`
- `#STOP`

### Respuestas esperadas del ESP32
- `#ACK,CONNECTION` o `#ACK,CONECTION`
- `#ACK,Battery`
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
Si solo quieres usar batView, abre el script de instalación de tu sistema operativo. Si quieres construirlo desde código, sigue estos pasos:

1. Abre una terminal dentro de la carpeta del proyecto.
2. Instala las herramientas base de tu sistema.
3. Ejecuta el script de tu sistema operativo.

### Comandos para instalar lo necesario
#### macOS

```bash
xcode-select --install
brew install cmake wxwidgets python pkg-config
```

#### Ubuntu / Debian
```bash
sudo apt update
sudo apt install -y cmake g++ make pkg-config libwxgtk3.2-dev libgtk-3-dev python3 python3-dev python3-pip
```

#### Windows
Instala estos programas:

- Visual Studio 2022 con `Desktop development with C++`
- `CMake`
- `Python 3`
- `wxWidgets` compatible con tu compilador

### Script para construir la app
#### macOS

```bash
./install_macos.sh
```

#### Linux
```bash
./install_linux.sh
```

#### Windows
```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1
```

### Si ya tienes las herramientas instaladas
```bash
./install_macos.sh --skip-deps
./install_linux.sh --skip-deps
```

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -SkipDeps
```

### Limpiar artefactos generados
Si quieres borrar lo que generan los scripts sin tocar el codigo fuente:

```bash
./build_app.sh --clean
```

En Windows:

```powershell
powershell -ExecutionPolicy Bypass -File .\build_app.ps1 --clean
```

Esto elimina `build-release/`, `dist/`, `python/runtime/` y la copia de `batView.app` en el Desktop.

### Para dejar la app fuera del repo
Después de compilar:

- macOS: el script copia `batView.app` al `Desktop`.
- Linux: copia `build-release/batView` y la carpeta `assets/` a una carpeta fuera del repo, por ejemplo `~/Apps/batView`.
- Windows: usa el `.exe` o el paquete portable que quede en `dist/`.

### App wxWidgets
Esta sección es la ruta manual avanzada. Solo úsala si quieres compilar paso a paso sin el script automático.

#### macOS
```bash
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --config Release
open build-release/batView.app
```

#### Linux
```bash
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON -DCMAKE_BUILD_TYPE=Release
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
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --config Release
.\build-release\Release\batView.exe
```

### Empaquetado manual
Comando recomendado para generar un paquete local portable:

```bash
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON -DCMAKE_BUILD_TYPE=Release -DBATVIEW_PACKAGE_FORMAT=PORTABLE
cmake --build build-release --config Release --target package
```

Esto genera artefactos dentro de `dist/` usando un formato portable y estable por plataforma:
- macOS: `.zip`
- Windows: `.zip`
- Linux: `.tar.gz`

Si necesitas un instalador nativo en vez del paquete portable, reconfigura con:
```bash
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON -DCMAKE_BUILD_TYPE=Release -DBATVIEW_PACKAGE_FORMAT=NATIVE
cmake --build build-release --config Release --target package
```

Notas:
- En macOS, el formato nativo usa `DragNDrop` (`.dmg`) y puede fallar en algunos entornos locales.
- En Linux, el formato nativo intenta generar un `.deb`.
- En Windows, el formato nativo intenta generar un instalador `NSIS`.

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
- [docs/BUILD_INSTALLER.md](./docs/BUILD_INSTALLER.md)

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
# batView

batView es una app de escritorio para controlar pruebas de baterías con un ESP32 por USB.

## Lo que necesitas

### macOS

```bash
xcode-select --install
brew install cmake wxwidgets python pkg-config
```

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install -y cmake g++ make pkg-config libwxgtk3.2-dev libgtk-3-dev python3 python3-dev python3-pip
```

### Windows

Instala estos programas:

- Visual Studio 2022 con `Desktop development with C++`
- `CMake`
- `Python 3`
- `wxWidgets` compatible con tu compilador

## Cómo construir la app

Usa solo el script de tu sistema operativo:

### macOS

```bash
./install_macos.sh
```

### Linux

```bash
./install_linux.sh
```

### Windows

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1
```

Si ya instalaste las herramientas antes, puedes saltarte esa parte:

```bash
./install_macos.sh --skip-deps
./install_linux.sh --skip-deps
```

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1 -SkipDeps
```

## Cómo usar la app fuera del repo

Después de correr el script:

- macOS: arrastra `build-release/batView.app` a `Applications`.
- Linux: copia `build-release/batView` y la carpeta `assets/` a una carpeta fuera del repo, por ejemplo `~/Apps/batView`.
- Windows: usa el `.exe` o el paquete que quede en `dist/`.

## Si algo falla

- Asegúrate de estar dentro de la carpeta `batView`.
- Si cambias de computadora o de sistema operativo, borra `build-release` y vuelve a correr el script.
- Si `wxWidgets` no se encuentra, instala el paquete de desarrollo para tu sistema y vuelve a intentar.
