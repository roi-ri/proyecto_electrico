# Arquitectura Lógica por Capas

Se propone una arquitectura basada en separación por capas (*layered architecture*), con el objetivo de garantizar modularidad, mantenibilidad y escalabilidad del sistema.

La arquitectura se divide en tres capas principales:

- Capa de Presentación  
- Capa de Lógica de Negocio  
- Capa de Datos  

---

## 1. Capa de Presentación (Presentation Layer)

### Responsabilidad

Encargada de la interacción directa con el usuario.

### Funciones principales

- Visualización de variables eléctricas en tiempo real.
- Representación gráfica de datos históricos.
- Configuración de parámetros de carga, descarga y ciclado.
- Selección de tipo de batería.
- Visualización de indicadores como SoH.
- Gestión de eventos de usuario.

### Implementación propuesta

Desarrollada en C++ (por ejemplo, utilizando un framework gráfico como Qt) para priorizar:

- Rendimiento
- Control de recursos
- Integración eficiente con la capa de comunicación

Esta capa no debe contener lógica de cálculo compleja ni acceso directo a hardware.

---

## 2. Capa de Lógica de Negocio (Business Logic Layer)

### Responsabilidad

Contiene las reglas del sistema y el procesamiento intermedio.

### Funciones principales

- Gestión del protocolo de comunicación con el BMS.
- Validación de parámetros operativos.
- Procesamiento de datos adquiridos.
- Cálculo de indicadores como State of Health (SoH).
- Coordinación entre adquisición de datos y almacenamiento.
- Gestión de estados del sistema (idle, carga, descarga, error).

### Implementación propuesta

Se propone un modelo híbrido:

- Núcleo en C++ para comunicación y control operativo.
- Módulos analíticos en Python para:
  - Cálculo estadístico.
  - Análisis comparativo.
  - Postprocesamiento de datos.
  - Generación de reportes.

La integración puede realizarse mediante:
- Embedding del intérprete de Python en C++.
- Comunicación interproceso (IPC).
- Enlace mediante bibliotecas como `pybind11`.

---

## 3. Capa de Datos (Data Layer)

### Responsabilidad

Encargada de la persistencia, almacenamiento e intercambio de información.

### Funciones principales

- Almacenamiento estructurado de datos experimentales.
- Exportación a formatos estándar (CSV, MAT, XLSX).
- Importación de datos históricos.
- Gestión de archivos de configuración.

### Consideraciones

- Separación clara entre datos crudos y datos procesados.
- Estructura que garantice trazabilidad experimental.
- Independencia del formato interno respecto a la visualización.
- Posibilidad de expansión futura hacia bases de datos estructuradas.

---

# Flujo General del Sistema

## Flujo principal

```shell
Usuario
↓
Capa de Presentación
↓
Capa de Lógica de Negocio
↓
Capa de Datos
↓
BMS / Ciclador
```


## Flujo para análisis avanzado

```shell
Datos adquiridos
↓
Módulo Python (procesamiento)
↓
Resultados (SoH, análisis)
↓
Interfaz gráfica
```



---

# Beneficios de esta Arquitectura

- Separación clara de responsabilidades.
- Reducción de acoplamiento entre interfaz y procesamiento.
- Mayor mantenibilidad.
- Posibilidad de reemplazar o actualizar módulos sin afectar todo el sistema.
- Escalabilidad futura hacia arquitectura más distribuida si fuera necesario.
- Flexibilidad para incorporar nuevos algoritmos de análisis sin modificar la capa de control.

