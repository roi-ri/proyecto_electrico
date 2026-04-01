# Propuesta de Arquitectura y Estructura del Repositorio  
## Aplicación de monitoreo y adquisición de datos vía ESP32

---

## 1. Introducción

El presente documento describe la arquitectura y la estructura propuesta para el desarrollo de una aplicación de escritorio basada en C++ y wxWidgets en macOS. La aplicación tiene como propósito principal la adquisición, procesamiento, visualización y almacenamiento de datos provenientes de un dispositivo ESP32 mediante comunicación USB (UART).

El sistema contempla:
- Comunicación bidireccional con el dispositivo
- Definición de un protocolo de comunicación propio
- Procesamiento estructurado de datos
- Exportación en formato CSV
- Integración de Python embebido para análisis y graficación
- Interfaz gráfica robusta y modular

---

## 2. Enfoque arquitectónico

Se plantea una arquitectura modular basada en separación de responsabilidades, estructurada en las siguientes capas:

### 2.1 Presentación (UI)
Encargada de la interacción con el usuario mediante wxWidgets. No contiene lógica de negocio ni acceso directo a hardware.

### 2.2 Aplicación (Application Layer)
Coordina los casos de uso del sistema y conecta la interfaz con los servicios del núcleo.

### 2.3 Núcleo (Core / Domain)
Contiene los modelos, reglas de negocio y definición del protocolo de comunicación.

### 2.4 Infraestructura
Implementa la interacción con elementos externos:
- Comunicación serial
- Sistema de archivos
- Exportación CSV
- Motor Python embebido
- Configuración
- Registro de eventos (logging)

---

## 3. Estructura del repositorio

```text
repo/
├── docs/
├── config/
├── python/
│   └── embedded/
├── tests/
├── src/
│   ├── app/
│   ├── core/
│   │   ├── models/
│   │   ├── protocol/
│   │   └── services/
│   ├── infrastructure/
│   │   ├── serial/
│   │   ├── csv/
│   │   ├── python/
│   │   ├── logging/
│   │   └── config/
│   └── ui/
│       ├── frames/
│       ├── panels/
│       ├── dialogs/
│       └── viewmodels/
├── data/
├── logs/
└── CMakeLists.txt
```

---

## 4. Descripción de componentes

### 4.1 src/main.cpp
Archivo de entrada del sistema. Se limita a:
- Inicialización de la aplicación
- Creación del contexto global
- Lanzamiento de la interfaz principal

---

### 4.2 app/
Contiene la lógica de coordinación global.

- Application: orquestador principal  
- AppContext: contenedor de dependencias  
- AppState: estado global del sistema  

---

### 4.3 core/

#### models/
Define las estructuras de datos principales:
- Measurement  
- Session  
- Command  
- Response  
- DeviceInfo  

#### protocol/
Define el protocolo de comunicación:
- Parseo de mensajes  
- Serialización  
- Validación de tramas  
- Manejo de errores  

#### services/
Contiene los casos de uso:
- DeviceService  
- AcquisitionService  
- SessionService  
- ExportService  
- PlotService  

---

### 4.4 infrastructure/

#### serial/
Gestión de comunicación USB:
- Apertura y cierre de puerto  
- Lectura continua  
- Escritura  
- Descubrimiento de puertos  

#### csv/
Lectura y escritura de archivos CSV.

#### python/
Integración con Python embebido:
- Inicialización del intérprete  
- Ejecución de scripts  
- Conversión de datos  

#### logging/
Registro de eventos del sistema.

#### config/
Carga de configuraciones desde archivos externos.

---

### 4.5 ui/

#### frames/
Ventanas principales.

#### panels/
Componentes reutilizables de la interfaz:
- Conexión  
- Adquisición  
- Visualización  
- Logs  

#### dialogs/
Ventanas auxiliares.

#### viewmodels/
Intermediarios entre la UI y la lógica del sistema.

---

## 5. Flujo de operación

1. La interfaz solicita conexión  
2. Se inicializa el puerto serial  
3. Se reciben datos del ESP32  
4. Se parsean mediante el módulo de protocolo  
5. Se transforman en modelos de datos  
6. Se almacenan en sesión activa  
7. Se exportan a CSV  
8. Se procesan mediante Python  
9. Se actualiza la interfaz gráfica  

---

## 6. Estructura de datos

Formato CSV sugerido:

```text
timestamp,sequence,channel,voltage,current,temperature,status,raw_frame
```

Estructura de almacenamiento:

```text
data/exports/
└── session_x/
    ├── raw_data.csv
    ├── processed_data.csv
    ├── metadata.json
    └── plots/
```

---

## 7. Integración con Python

Los scripts se organizan en:

```text
python/embedded/
```

Ejemplos:
- plotter.py  
- filters.py  
- transforms.py  

---

## 8. Configuración

Archivo base:

```json
{
  "serial": {
    "baudrate": 115200,
    "timeout_ms": 200
  },
  "paths": {
    "exports_dir": "./data/exports"
  }
}
```

---

## 9. Pruebas

Tipos de pruebas:

- Unitarias: parseo, modelos, CSV  
- Integración: flujo completo de datos  
- Protocolo: validación de tramas  

---

## 10. Documentación

Ubicada en `docs/`:

- architecture.md  
- protocol.md  
- data-format.md  

---

## 11. Conclusión

La estructura planteada permite una separación clara de responsabilidades, facilitando la escalabilidad, mantenibilidad y robustez del sistema. La organización modular asegura que cada componente evolucione de manera independiente, manteniendo una base sólida para futuras extensiones.

