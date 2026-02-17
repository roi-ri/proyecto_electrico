# Propuesta para el Software y visualización/control de datos

## Objetivo del sistema

Desarrollar una aplicación de software orientada a la adquisición, visualización y análisis de datos provenientes de un sistema de gestión de baterías (BMS) mediante un protocolo de comunicación definido, que permita:
- Visualizar variables eléctricas y operativas en tiempo real mediante gráficas y medidores.
- Controlar y configurar modos de operación para carga, descarga y ciclado de baterías.
- Seleccionar y parametrizar distintos tipos de baterías según sus características nominales.
- Exportar e importar datos en formatos estándar (CSV, MAT, XLSX) para análisis posterior.
- Estimar y mostrar indicadores de estado de la batería, incluyendo State of Health (SoH). El cálculo y los diversos aspectos necesarios se verán estipulados por el grupo asociado al ciclador.


## Usuario Objetivo

El sistema está orientado a usuarios que requieran estudiar y analizar el comportamiento de baterías, tales como estudiantes, investigadores o personal técnico en laboratorio.
La aplicación permitirá monitorear variables eléctricas relevantes, controlar parámetros operativos de carga y descarga, y evaluar indicadores de vida útil. Asimismo, brindará la posibilidad de analizar datos históricos previamente almacenados para realizar estudios comparativos o evaluaciones de degradación.


## Problemas que resuelve

Actualmente, el estudio y monitoreo de baterías en entornos académicos o de laboratorio suele depender de herramientas fragmentadas, interfaces limitadas del propio ciclador o procesamiento manual de datos, lo que genera las siguientes problemáticas:
- Falta de visualización integrada en tiempo real: Las variables eléctricas (voltaje, corriente, temperatura, capacidad, etc.) no siempre pueden observarse de forma centralizada y dinámica.
- Limitado control parametrizable: La configuración de modos de carga, descarga o ciclado puede estar restringida a interfaces poco flexibles o no adaptadas a distintos tipos de batería.
- Dificultad para estimar el estado de salud (SoH): La evaluación de degradación suele requerir procesamiento externo o cálculos manuales, reduciendo la trazabilidad del análisis.
- Procesamiento manual de datos históricos: La manipulación de archivos experimentales sin una herramienta dedicada incrementa el riesgo de errores y dificulta la comparación entre ensayos.
- Falta de estandarización en exportación y análisis: La ausencia de formatos estructurados o compatibles con herramientas de análisis técnico limita la reproducibilidad de resultados.
- El sistema propuesto centraliza la adquisición, control y análisis de datos en una única plataforma, mejorando la trazabilidad experimental, reduciendo errores operativos y facilitando estudios comparativos de desempeño y degradación de baterías.

## Contexto de uso 

El sistema está diseñado para operar en entornos académicos, de investigación o laboratorio técnico donde se realicen pruebas controladas de carga, descarga y ciclado de baterías mediante un sistema de gestión de baterías (BMS) o ciclador compatible.
La aplicación funcionará como interfaz de supervisión y control, comunicándose con el hardware a través de un protocolo de comunicación previamente definido. Su uso está orientado a pruebas experimentales en condiciones controladas y no contempla, en esta etapa, aplicaciones industriales de alta potencia ni integración directa en sistemas críticos de producción.
El software será utilizado en equipos de cómputo convencionales y dependerá de la correcta configuración del sistema de adquisición y comunicación con el dispositivo externo.


## Alcances
El sistema contempla el desarrollo de una aplicación de software con las siguientes capacidades:
- Comunicación con un BMS o ciclador mediante un protocolo previamente definido.
- Adquisición y visualización en tiempo real de variables eléctricas y operativas.
- Configuración de parámetros de carga, descarga y ciclado dentro de los límites establecidos por el hardware.
- Gestión y almacenamiento estructurado de datos experimentales.
- Exportación e importación de datos en formatos estándar (CSV, MAT, XLSX).
- Visualización y estimación de indicadores de desempeño y degradación, incluyendo el State of Health (SoH), según los criterios definidos por el grupo responsable del ciclador.
- Análisis comparativo de datos históricos almacenados.

## Limitaciones
El sistema no contempla en esta etapa:
- Desarrollo o modificación del hardware del BMS o ciclador.
- Implementación de algoritmos avanzados de modelado electroquímico fuera de los definidos por el equipo técnico.
- Integración con plataformas industriales de control en tiempo real o sistemas críticos.
- Operación en entornos de alta potencia sin validación adicional.
- Certificación bajo normativas industriales específicas.
- El desempeño y precisión del sistema dependerán de la calidad de los datos proporcionados por el hardware y de la correcta implementación del protocolo de comunicación.



## Posibles Arquitecturas del Sistema
### 1. Arquitectura Monolítica en C/C++
#### Descripción:
En esta alternativa, todos los componentes del sistema (interfaz gráfica, comunicación con el BMS, lógica de control, procesamiento y almacenamiento de datos) se desarrollan en C o C++.
#### Ventajas: 
- Alto rendimiento y control sobre recursos.
- Menor dependencia de entornos externos.
- Despliegue simplificado (aplicación unificada).
- Mayor determinismo en operaciones críticas.
#### Desventajas: 
- Mayor complejidad en desarrollo de herramientas analíticas avanzadas.
- Menor flexibilidad para prototipado rápido.
- Incremento en tiempo de desarrollo para módulos de análisis matemático.
--------
### 2. Arquitectura Híbrida C++ + Python (Propuesta Principal)
#### Descripción: 
Se propone una arquitectura híbrida en la que:
- C/C++ gestiona la comunicación con el hardware, la interfaz gráfica y el control operativo.
- Python se encarga del procesamiento numérico, análisis de datos, generación de reportes y visualización avanzada.
- Distribución de responsabilidades
##### C/C++:
- Implementación del protocolo de comunicación con el BMS.
- Adquisición de datos en tiempo real.
- Control de modos de carga, descarga y ciclado.
- Gestión de la interfaz gráfica principal.
##### Python:
- Procesamiento estadístico y cálculo de indicadores (ej. SoH).
- Análisis comparativo de datos históricos.
- Exportación avanzada de datos.
- Generación de gráficas especializadas.
- Mecanismos de integración posibles
- Embedding de intérprete Python en C++.
- Comunicación mediante IPC (sockets o pipes).
- Uso de bibliotecas de enlace como pybind11.
#### Ventajas
- Optimización de tareas críticas en C/C++.
- Flexibilidad y rapidez de desarrollo en análisis científico con Python.
- Separación clara entre control operativo y análisis.
#### Desventajas
- Mayor complejidad de integración.
- Gestión adicional de dependencias.
- Necesidad de definir claramente los límites entre módulos.

---------------
### 3. Arquitectura Modular por Servicios
#### Descripción
Separación del sistema en servicios independientes:
- Servicio de adquisición (C++).
- Servicio de procesamiento (Python).
- Interfaz de usuario independiente.
- La comunicación puede realizarse mediante API local o mecanismos de mensajería.
#### Ventajas
- Alta escalabilidad.
- Modularidad avanzada.
- Mayor mantenibilidad a largo plazo.
#### Desventajas
- Mayor complejidad estructural.
- Posible sobredimensionamiento para entornos académicos.

-----------
### Arquitectura Recomendada

Se propone la arquitectura híbrida C++ + Python como solución principal, debido a que:
- Permite optimizar el rendimiento en tareas críticas.
- Aprovecha el ecosistema científico de Python para análisis.
- Es adecuada para entornos académicos y de investigación.
- Ofrece un balance entre eficiencia y flexibilidad.


Se da una descripción de la arquitectura lógica por capas: [`ALC`](/Arquitectura_Logica_Capas.md)
