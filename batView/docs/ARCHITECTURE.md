# Arquitectura

## Visión general

batView está organizado en capas para separar interfaz, lógica de protocolo e infraestructura de hardware.

```text
UI -> ViewModel -> Services -> Protocol / Models -> Infrastructure
```

## Capas

### UI

Responsable de:

- eventos de usuario
- layout y navegación
- presentación del estado
- tráfico visible de mensajes

Archivos principales:

- `src/ui/BatViewApp.cpp`
- `src/ui/frames/MainFrame.cpp`
- `src/ui/panels/ConnectionPanel.cpp`
- `src/ui/dialogs/ExportDialog.cpp`

### ViewModel

Responsable de:

- exponer operaciones de alto nivel a la UI
- coordinar servicios sin acoplar la ventana a infraestructura concreta

Archivo principal:

- `src/ui/viewmodels/MainViewModel.cpp`

### Core

Responsable de:

- reglas del flujo de conexión
- validación del protocolo
- adquisición de telemetría
- exportación lógica

Módulos:

- `protocol/`
- `services/`
- `models/`

### Infrastructure

Responsable de:

- acceso serial real
- logging
- exportación concreta

Módulos:

- `serial/`
- `logging/`
- `csv/`

## Flujo interno

### Conexión

1. `MainFrame` solicita conexión al `MainViewModel`.
2. `MainViewModel` delega en `ConnectionService`.
3. `ConnectionService` usa `ISerialPort`.
4. Se abre el puerto y se intenta `#CONNECTION`.
5. Si el ESP32 reinicia al abrir, el servicio reintenta el handshake.
6. Si llega `#ACK,CONNECTION`, la UI pasa a estado conectado.

### Operación

1. La UI envía selección de batería.
2. `ProtocolSessionService` valida `ACK`.
3. Según la función elegida, envía `#LOAD`, `#UNLOAD` o `#CICLE`.
4. Si el comando se confirma, `AcquisitionService` comienza a leer telemetría.
5. La UI puede detener cualquier operación con `#STOP`.

### Adquisición

1. `AcquisitionService` lee líneas del serial en un hilo dedicado.
2. `TextMessageParser` intenta convertir tramas `#DATA` en `Measurement`.
3. Las mediciones válidas quedan en memoria para exportación o graficación futura.

## Decisiones de diseño

- La UI no accede directamente al puerto serial.
- El protocolo está centralizado en `TextProtocolCodec`.
- El parser de telemetría está separado del envío de comandos.
- El acceso serial real se abstrae detrás de `ISerialPort`.
- Los errores seriales de sistema se conservan para diagnóstico.

## Estado de arquitectura

La base actual es adecuada para:

- validación funcional con ESP32 real
- expansión de la UI
- aumento de cobertura de tests
- incorporación de exportadores adicionales

Las principales mejoras futuras serían:

- pruebas de integración del flujo UI-servicios
- soporte serial nativo para Windows
- gráfica real
- almacenamiento persistente de sesiones
