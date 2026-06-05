# batView

batView es una app de escritorio para controlar pruebas de baterías con un ESP32 por USB.

La idea es simple: abrir la app, conectar el ESP32 y empezar a trabajar sin tener que saber Git ni compilar código.

## Resumen

batView sirve para ver datos del ESP32 en tiempo real, controlar la prueba desde una interfaz gráfica y guardar los resultados para revisarlos después.

## Características principales

- Conecta con un ESP32 por USB.
- Reintenta la conexión si el ESP32 reinicia al abrir el puerto.
- Muestra los datos recibidos en tiempo real.
- Guarda el registro de lo que pasa entre el ESP32 y la PC.
- Exporta mediciones y gráficas a CSV, MAT, XLSX y PNG.
- Incluye ejemplos para Arduino IDE y PlatformIO.

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
│   ├── DOWNLOAD_AND_INSTALL.md       centro de descarga/instalacion
│   ├── DOWNLOAD_WINDOWS.md           guia de instalacion en Windows
│   ├── DOWNLOAD_MACOS.md             guia de instalacion en macOS
│   ├── DOWNLOAD_LINUX.md             guia de instalacion en Linux
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

### Antes de instalar o compilar

Si solo quieres usar batView, no necesitas compilar nada. Ve a [docs/DOWNLOAD_AND_INSTALL.md](./docs/DOWNLOAD_AND_INSTALL.md).

Si quieres construir la app desde el código, usa la rama `batView`.

```bash
git checkout batView
```

Todos los comandos se ejecutan dentro de la carpeta del proyecto, donde están `README.md`, `CMakeLists.txt` y los scripts de instalación.

```bash
cd batView
```

### Uso recomendado: un solo comando

Si quieres compilar la aplicación completa y generar el paquete para tu sistema, usa este flujo.

El script hace esto automáticamente:

- detecta el sistema operativo;
- prepara un runtime local de Python embebido;
- instala las dependencias de Python necesarias para `XLSX` y `PNG`;
- configura y compila `batView`;
- genera el paquete apropiado para el sistema actual;
- abre la app al final, a menos que uses `--no-run`.

#### macOS y Linux

```bash
./build_app.sh
```

#### Windows

```powershell
powershell -ExecutionPolicy Bypass -File .\build_app.ps1
```

#### Si no quieres abrir la app al terminar

```bash
./build_app.sh --no-run
```

### Instalación automática de herramientas y empaquetado

Si quieres preparar una computadora desde cero y generar el paquete final, usa el script de tu sistema operativo.

Windows, desde PowerShell como administrador:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1
```

macOS o Linux, desde Terminal:

```bash
./install_macos.sh
./install_linux.sh
```

El resultado aparece en `dist/`. La guía paso a paso para personas que no conocen el proyecto está en [docs/BUILD_INSTALLER.md](./docs/BUILD_INSTALLER.md).

### Qué necesitas instalar antes

El script automatiza casi todo, pero necesitas algunas herramientas base instaladas en tu sistema.

#### macOS

Necesitas instalar:

- `Xcode Command Line Tools`
- `CMake`
- `wxWidgets`
- `Python 3`

Comandos sugeridos:

```bash
xcode-select --install
brew install cmake wxwidgets python
```

#### Ubuntu / Debian

Necesitas instalar:

- `cmake`
- compilador `g++`
- `make`
- `pkg-config`
- `libwxgtk3.2-dev`
- `libgtk-3-dev`
- `python3`
- `python3-pip`

Comandos sugeridos:

```bash
sudo apt update
sudo apt install -y cmake g++ make pkg-config libwxgtk3.2-dev libgtk-3-dev python3 python3-pip
```

Si `wxWidgets` no aparece correctamente:

```bash
sudo apt install -y wx3.2-headers wx-common
```

#### Windows

Necesitas instalar:

- Visual Studio 2022 con `Desktop development with C++`
- `CMake`
- `Python 3`
- `wxWidgets` compatible con tu toolchain

Notas:

- En Windows el script también detecta el OS y ejecuta el flujo completo, pero la disponibilidad del instalador nativo depende de las herramientas instaladas en la máquina.
- Si no hay generador nativo disponible, el flujo cae a un paquete portable.

### Qué no tienes que instalar manualmente

El script instala automáticamente estas dependencias de Python dentro del proyecto:

- `Pillow`
- `openpyxl`

No se instalan globalmente. Se guardan en:

- `python/runtime/`

### ¿Se necesita internet?

Sí, la primera vez normalmente necesitas internet para descargar:

- `Pillow`
- `openpyxl`

Después de la primera vez, el flujo reutiliza lo que ya quedó preparado en `python/runtime/`.

### Qué genera al final

Después de correr el comando:

- la app compilada queda en `build-release/`;
- el paquete final queda en `dist/`;
- el runtime embebido usado por la app queda en `python/runtime/`.

Formatos esperados:

- macOS: `.app` y `.zip`
- Linux: binario ejecutable y `.tar.gz` o `.deb` según herramientas disponibles
- Windows: `.exe` y paquete portable o instalador según herramientas disponibles

### Proceso recomendado paso a paso

1. Entra a la carpeta `batView`.
2. Instala las herramientas base de tu sistema una sola vez.
3. Ejecuta el comando único.
4. Espera a que termine la compilación.
5. Abre la app generada o toma el paquete desde `dist/`.

Ejemplo en macOS/Linux:

```bash
cd batView
./build_app.sh
```

Ejemplo en Windows:

```powershell
cd batView
powershell -ExecutionPolicy Bypass -File .\build_app.ps1
```

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
- [docs/DOWNLOAD_AND_INSTALL.md](./docs/DOWNLOAD_AND_INSTALL.md)
- [docs/DOWNLOAD_WINDOWS.md](./docs/DOWNLOAD_WINDOWS.md)
- [docs/DOWNLOAD_MACOS.md](./docs/DOWNLOAD_MACOS.md)
- [docs/DOWNLOAD_LINUX.md](./docs/DOWNLOAD_LINUX.md)

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
