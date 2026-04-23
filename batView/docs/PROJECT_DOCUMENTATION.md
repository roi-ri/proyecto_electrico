# Documentación del Proyecto

## Resumen

batView es una aplicación de escritorio para supervisar pruebas de baterías controladas por un ESP32.  
La aplicación se comunica por puerto serial, valida un protocolo simple basado en texto y presenta un flujo guiado de operación en la UI.

## Objetivos funcionales

- Establecer conexión con un ESP32 por USB/UART.
- Confirmar handshake mediante `ACK`.
- Enviar comandos de selección de batería y modo de trabajo.
- Iniciar operaciones de carga, descarga o ciclado.
- Detener cualquier operación activa mediante `#STOP`.
- Recibir y almacenar telemetría.
- Exportar resultados a CSV.
- Exportar resultados completos a MAT y XLSX.
- Exportar gráficas derivadas a CSV, MAT, XLSX y PNG.

## Flujo de usuario

1. Seleccionar puerto serial.
2. Conectar con el ESP32.
3. Elegir tipo de batería.
4. Elegir función:
   - Carga
   - Descarga
   - Ciclado
5. Configurar porcentaje o ciclos según corresponda.
6. Iniciar operación.
7. Detener cuando sea necesario.
8. Exportar datos si se desea.

## Flujo de comunicación

### Conexión

- App: `#CONNECTION`
- ESP32: `#ACK,CONNECTION` o `#ACK,CONECTION`
- Estado adicional aceptado: `#STATUS,CONNECTED,ESP32_READY`

La app reintenta el handshake automáticamente para tolerar ESP32 que reinician al abrir el puerto.

### Selección de batería

- App: `#DATA,1,<tipoBateria>,<funcion>`
- ESP32: `#ACK,DATA`

### Inicio de operación

- Carga:
  - App: `#LOAD,<0-100>`
  - ESP32: `#ACK,LOAD`
- Descarga:
  - App: `#UNLOAD,<0-100>`
  - ESP32: `#ACK,UNLOAD`
- Ciclado:
  - App: `#CICLE,0,0` o `#CICLE,1,<n>`
  - ESP32: `#ACK,CICLE`

### Detención

- App: `#STOP`
- ESP32: `#ACK,STOP`

### Telemetría

Se soportan dos variantes:

- `#DATA,<voltaje>,<corriente>,<timestamp>`
- `#DATA,<voltaje>,<corriente>,<timestamp>,<estado>,<ciclos>`

#### Origen y uso de los campos

Todos los campos de telemetría usados por la aplicación provienen del ESP32:

- `voltaje`
- `corriente`
- `timestamp`
- `estado`
- `ciclos`

La aplicación parsea esas tramas y las convierte a la estructura `Measurement` para conservarlas en memoria, exportarlas y graficarlas.

En particular:

- `timestamp` es esperado desde el ESP32 en milisegundos
- para la visualización de gráficas, la app convierte ese valor a segundos
- esa conversión solo afecta la presentación del eje X, no el valor base recibido ni el dato almacenado

## Manejo de errores

La aplicación distingue entre:

- fallo al abrir el puerto serial
- fallo al configurar el puerto
- fallo al escribir en serial
- timeout esperando `ACK`
- error explícito enviado por el ESP32

Cuando el backend serial reporta un error del sistema, la app conserva el detalle para facilitar diagnóstico.

## Componentes principales

### UI

- `src/ui/BatViewApp.*`
  Punto de entrada wxWidgets.

- `src/ui/frames/MainFrame.*`
  Ventana principal y flujo operativo.

- `src/ui/panels/ConnectionPanel.*`
  Selector de puerto y estado de conexión.

- `src/ui/dialogs/ExportDialog.*`
  Diálogo de exportación.

### Servicios

- `src/core/services/ConnectionService.*`
  Conexión serial y handshake.

- `src/core/services/ProtocolSessionService.*`
  Envío de comandos con espera de `ACK`.

- `src/core/services/AcquisitionService.*`
  Lectura de telemetría en segundo plano.

- `src/core/services/ExportService.*`
  Exportación por formato.

### Protocolo

- `src/core/protocol/TextProtocolCodec.*`
  Construcción y validación de mensajes del protocolo.

- `src/core/protocol/TextMessageParser.*`
  Conversión de tramas `#DATA` a mediciones.

### Infraestructura

- `src/infrastructure/serial/PosixSerialPort.*`
  Implementación serial real para entornos POSIX.

- `src/infrastructure/serial/Win32SerialPort.*`
  Implementación serial real para Windows.

- `src/infrastructure/serial/SerialPortStub.*`
  Implementación simulada para modo consola y pruebas tempranas.

- `src/infrastructure/csv/CsvExporter.*`
  Exportación CSV.

- `src/infrastructure/logging/ConsoleLogger.*`
  Logging a consola.

- `src/infrastructure/python/PythonPlotEngine.*`
  Render de gráficas y exportación XLSX mediante Python embebido cuando está disponible.

## Build y ejecución

### App principal

### macOS

```bash
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON
cmake --build build-release --config Release
open build-release/batView.app
```

### Linux

```bash
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON
cmake --build build-release --config Release
./build-release/batView
```

### Windows

```powershell
cmake -S . -B build-release -DBATVIEW_ENABLE_WX=ON
cmake --build build-release --config Release
.\build-release\Release\batView.exe
```

### Empaquetado para distribución

```bash
cmake --build build-release --config Release --target package
```

El artefacto final se genera en `dist/`, fuera de la carpeta `build-release/`.

### Tests

```bash
cmake -S . -B build-tests -DBATVIEW_BUILD_TESTS=ON
cmake --build build-tests --config Release
ctest --test-dir build-tests --output-on-failure
```

## Ejemplos de hardware

Hay proyectos de prueba listos para subir al ESP32 en:

- `examples/arduino/esp32_batview_test`
- `examples/platformio/esp32_batview_test`
