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
│   ├── BUILD_INSTALLER.md            indice de instalacion y limpieza
│   ├── INSTALL_MACOS.md              instalacion en macOS
│   ├── INSTALL_LINUX.md              instalacion en Linux
│   ├── INSTALL_WINDOWS.md            instalacion en Windows
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
If you want to install batView, open the guide for your system:

- [macOS install guide](docs/INSTALL_MACOS.md)
- [Linux install guide](docs/INSTALL_LINUX.md)
- [Windows install guide](docs/INSTALL_WINDOWS.md)

Each guide has two clear cases:

- If nothing is installed yet.
- If everything needed is already installed.

Use only the guide for your own computer.

| Your computer | Script to run |
|---|---|
| Mac | `./install_macos.sh` |
| Linux | `./install_linux.sh` |
| Windows | `powershell -ExecutionPolicy Bypass -File .\install_windows.ps1` |

Do not run the other scripts. Each one is for a different operating system.

### What you need before installing
If you are not sure what to install first, use this checklist.

#### Mac
Install these once if they are not already installed:

1. Apple command line tools.
2. Homebrew.
3. These Homebrew packages: `cmake`, `wxwidgets`, `python`, `pkg-config`.

Step by step:

```bash
xcode-select --install
```

Then install Homebrew if needed:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Then install the required packages:

```bash
brew install cmake wxwidgets python pkg-config
```

#### Linux
Install these packages:

1. CMake.
2. A C++ compiler and build tools.
3. wxWidgets development files.
4. GTK development files.
5. Python 3, Python headers, and pip.

Step by step on Ubuntu or Debian:

```bash
sudo apt update
sudo apt install -y cmake g++ make pkg-config libwxgtk3.2-dev libgtk-3-dev python3 python3-dev python3-pip
```

Step by step on Fedora:

```bash
sudo dnf install -y cmake gcc-c++ make pkgconf-pkg-config wxGTK-devel gtk3-devel python3 python3-devel python3-pip rpm-build zip
```

Step by step on Arch Linux:

```bash
sudo pacman -S --needed cmake gcc make pkgconf wxwidgets-gtk3 gtk3 python python-pip zip
```

#### Windows
Install these programs:

1. Visual Studio 2022 with the `Desktop development with C++` workload.
2. CMake.
3. Python 3.
4. wxWidgets compatible with your compiler.
5. Git.
6. NSIS if you want an installer package.

Step by step:

```powershell
winget --version
winget install --id Microsoft.VisualStudio.2022.BuildTools --exact --silent --accept-package-agreements --accept-source-agreements --override "--quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
winget install --id Kitware.CMake --exact --silent --accept-package-agreements --accept-source-agreements
winget install --id Python.Python.3.13 --exact --silent --accept-package-agreements --accept-source-agreements
winget install --id Git.Git --exact --silent --accept-package-agreements --accept-source-agreements
winget install --id NSIS.NSIS --exact --silent --accept-package-agreements --accept-source-agreements
```

Then run the project installer:

```powershell
cd C:\path\to\proyecto_electrico\batView
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1
```

### If the tools are already installed
Use the same script, but add the skip-deps option.

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
- Linux: el script crea un lanzador `batView.desktop` en el `Desktop`.
- Windows: el script crea un acceso directo `batView.lnk` en el `Desktop`.

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
