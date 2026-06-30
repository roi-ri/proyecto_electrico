# Proyecto Eléctrico - Ciclador de Baterías

Repositorio integral del proyecto de diseño, construcción y validación de un
ciclador de baterías de propósito múltiple. El proyecto reúne el hardware de
potencia y medición, el firmware de control, el protocolo de comunicación y una
aplicación de escritorio para operar pruebas, visualizar telemetría y exportar
resultados.

## Descripción general

El sistema está orientado a ejecutar pruebas controladas de carga, descarga y
ciclado sobre baterías. La arquitectura combina una PCB dedicada, un ESP32 como
unidad de control y adquisición, sensores de corriente y tensión, y la
aplicación `batView` para supervisar el proceso desde una computadora.

Los objetivos principales del repositorio son:

- Centralizar la documentación técnica y administrativa del proyecto.
- Mantener el diseño eléctrico y los archivos de fabricación de la PCB.
- Versionar el firmware encargado del control y la comunicación.
- Desarrollar la aplicación de escritorio para operación, monitoreo y
  exportación de datos.
- Conservar cronogramas, bitácoras y propuestas como evidencia del avance.

## Componentes del sistema

| Componente | Ubicación | Descripción |
|---|---|---|
| Aplicación de escritorio | `batView/` | Interfaz en C++/wxWidgets para controlar pruebas vía USB/UART, graficar telemetría y exportar resultados. |
| Firmware/control | `code/` | Código en C para control PI, monitoreo de carga/descarga, límites de tensión y comunicación con la PC. |
| PCB | `PCB/` | Proyecto KiCad con esquemático, layout, huellas, símbolos, modelos 3D, salidas e imágenes del diseño. |
| Propuesta | `propuesta/` | Descripción inicial del ciclador, diagramas y documentos de diseño por área. |
| Cronograma | `cronograma/` | Plan de trabajo semanal del proyecto. |
| Bitácoras | `bitácora/` | Registro individual de avances, decisiones y actividades. |

## Estructura del repositorio

```text
proyecto_electrico/
|
├── README.md                         documentación principal del repositorio
├── LICENSE                           licencia MIT del proyecto
├── .gitignore                        reglas generales de archivos ignorados
├── .gitmodules                       submódulos utilizados por el repositorio
|
├── batView/                          aplicación de escritorio del proyecto
│   ├── README.md                     guía técnica de batView
│   ├── CMakeLists.txt                configuración de compilación y pruebas
│   ├── build_app.sh                  build completo para macOS/Linux
│   ├── build_app.ps1                 build completo para Windows
│   ├── install_macos.sh              instalación y empaquetado en macOS
│   ├── install_linux.sh              instalación y empaquetado en Linux
│   ├── install_windows.ps1           instalación y empaquetado en Windows
│   ├── src/                          código fuente de la aplicación
│   ├── tests/                        pruebas unitarias
│   ├── docs/                         documentación de arquitectura e instalación
│   ├── examples/                     firmware de prueba para ESP32
│   ├── scripts/                      automatización de build
│   └── assets/                       iconos e imágenes de la aplicación
|
├── code/                             firmware y control embebido
│   ├── include/                      encabezados del firmware
│   └── src/                          implementaciones en C
|
├── PCB/                              diseño electrónico en KiCad
│   ├── README.md                     documentación de la PCB
│   ├── Proyecto.kicad_pro            archivo principal del proyecto KiCad
│   ├── Proyecto.kicad_sch            esquemático eléctrico
│   ├── Proyecto.kicad_pcb            layout de la PCB
│   ├── symbols/                      símbolos personalizados
│   ├── footprints/                   huellas personalizadas
│   ├── 3dmodels/                     modelos 3D de componentes
│   ├── outputs/                      salidas generadas del diseño
│   └── Imagenes/                     vistas y capturas del diseño
|
├── propuesta/                        propuesta y documentos iniciales
│   ├── propuesta.md                  planteamiento general
│   ├── images/                       diagramas e imágenes de apoyo
│   ├── programa_sw/                  documentación del software
│   ├── protocolo_comunicacion/       documentación del protocolo
│   └── ciclador/                     documentación del ciclador
|
├── cronograma/                       planificación temporal
│   └── cronograma.md                 cronograma semanal
|
└── bitácora/                         bitácoras individuales del equipo
    ├── Brandon.md
    ├── Edu.md
    ├── Jose.md
    ├── Luis.md
    └── Rodrigo.md
```

## Flujo de trabajo del sistema

1. La PCB integra las etapas de potencia, acondicionamiento, sensado y conexión
   con el ESP32.
2. El firmware del ESP32 ejecuta el control de carga/descarga, monitorea
   variables eléctricas y aplica límites de seguridad.
3. La PC se comunica con el ESP32 mediante USB/UART usando tramas de protocolo
   definidas para conexión, configuración, carga, descarga, ciclado y paro.
4. `batView` envía comandos, recibe telemetría, muestra el estado de la prueba,
   grafica mediciones y exporta datos para análisis posterior.

## Protocolo de comunicación

La aplicación y el ESP32 intercambian mensajes de texto con prefijo `#`. Entre
las tramas principales utilizadas por `batView` se encuentran:

- `#CONNECTION`
- `#Battery,<BatteryNameID>,<V@max>,<V@min>,<Amax>`
- `#CICLE,0,0`
- `#CICLE,1,<numeroCiclos>`
- `#LOAD,<0-100>`
- `#UNLOAD,<0-100>`
- `#STOP`
- `#DATA,<tensión>,<corriente>,<timestamp>`

La documentación detallada del protocolo y del flujo de la aplicación se
encuentra en [`batView/README.md`](batView/README.md) y en
[`propuesta/protocolo_comunicacion/`](propuesta/protocolo_comunicacion/).

## Requisitos principales

### Software de escritorio

Para compilar y ejecutar `batView` se requiere, según el sistema operativo:

- CMake.
- Compilador C++ compatible.
- wxWidgets.
- Python 3 y dependencias de empaquetado/exportación.
- Git.

Guías específicas:

- [Instalación en macOS](batView/docs/INSTALL_MACOS.md)
- [Instalación en Linux](batView/docs/INSTALL_LINUX.md)
- [Instalación en Windows](batView/docs/INSTALL_WINDOWS.md)

### Diseño electrónico

Para abrir o modificar la PCB se recomienda:

- KiCad.
- Librerías locales incluidas en `PCB/symbols/`, `PCB/footprints/` y
  `PCB/3dmodels/`.

### Firmware

Para trabajar con el código embebido se recomienda contar con:

- Toolchain compatible con ESP32 o entorno equivalente.
- Acceso al puerto USB/UART del microcontrolador.
- Instrumentación básica para validar tensión, corriente y polaridad antes de
  conectar baterías reales.

## Puesta en marcha rápida

1. Clonar el repositorio:

```bash
git clone <URL_DEL_REPOSITORIO>
cd proyecto_electrico
```

2. Revisar la documentación de la PCB:

```bash
open PCB/README.md
```

3. Entrar a la aplicación de escritorio:

```bash
cd batView
```

4. Ejecutar el instalador correspondiente al sistema:

```bash
./install_macos.sh
```

En Linux:

```bash
./install_linux.sh
```

En Windows:

```powershell
powershell -ExecutionPolicy Bypass -File .\install_windows.ps1
```

## Documentación recomendada

- [`batView/README.md`](batView/README.md): guía principal de la aplicación,
  arquitectura, protocolo, build, pruebas y exportación.
- [`PCB/README.md`](PCB/README.md): descripción de la PCB, conectores, sensores,
  capas y recomendaciones de conexión.
- [`propuesta/propuesta.md`](propuesta/propuesta.md): planteamiento general del
  proyecto.
- [`cronograma/cronograma.md`](cronograma/cronograma.md): actividades planeadas
  por semana.
- [`bitácora/`](bitácora/): seguimiento individual del avance del equipo.

## Estado del proyecto

El repositorio contiene una base funcional y documentada para el desarrollo del
ciclador:

- Aplicación `batView` con conexión serial, flujo guiado de pruebas, gráficas y
  exportación.
- Proyecto KiCad de PCB con documentación visual y descripción de conectores.
- Código de control con módulos separados para comunicación, control PI,
  monitoreo, límites de tensión y paro.
- Documentación académica de propuesta, cronograma y bitácoras.

## Buenas prácticas de uso

- Validar polaridad y niveles de tensión antes de conectar una batería real.
- Probar primero con fuentes limitadas en corriente o cargas controladas.
- Mantener separadas las salidas generadas localmente de los archivos fuente.
- Registrar cambios relevantes en las bitácoras correspondientes.
- Actualizar la documentación cuando se modifique el protocolo, la PCB o el
  flujo de operación de `batView`.

## Licencia

Este proyecto se distribuye bajo licencia MIT. Consulte [`LICENSE`](LICENSE)
para ver los términos completos.
