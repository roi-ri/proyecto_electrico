
# Propuesta del circuito ciclador

## Objetivo del circuito

El objetivo principal del proyecto es diseñar e implementar un circuito
ciclador capaz de cargar y descargar una batería mediante el control de la
corriente. Para ello, se utilizará una topología basada en inductores y
transistores de potencia, acompañada de un sistema de control que permita
regular adecuadamente los procesos de carga y descarga.

Los objetivos específicos del proyecto son los siguientes:

- Diseñar un circuito capaz de cargar y descargar una batería mediante el
  control de la corriente que circula a través del inductor.

- Seleccionar y dimensionar adecuadamente los componentes electrónicos que
  serán utilizados en el circuito.

- Diseñar un sistema de control basado en el comportamiento eléctrico del
  circuito, que permita regular la corriente de la batería de manera estable,
  suave y eficiente.

- Realizar simulaciones del circuito y del sistema de control en PLECS para
  validar su funcionamiento antes de la implementación física.

- Diseñar e implementar el circuito ciclador en una PCB utilizando KiCad.

- Ensamblar y soldar los componentes electrónicos sobre la PCB para construir
  el prototipo físico.

- Diseñar una interfaz que permita supervisar y configurar el funcionamiento
  del circuito desde una computadora.

- Estimar y cotizar el costo de los componentes electrónicos, la fabricación
  de la PCB y los demás elementos necesarios para construir el prototipo.

## Usuario objetivo

El sistema está orientado a estudiantes, investigadores y personal técnico de
laboratorio que requieran estudiar y analizar el comportamiento de baterías
mediante pruebas controladas de carga, descarga y ciclado.

El circuito permitirá conectar una batería y controlar sus procesos de carga y
descarga desde una computadora. Por medio de una interfaz de usuario, será
posible configurar los parámetros de la prueba, indicar el tipo de operación
que debe ejecutar el circuito y supervisar las principales variables eléctricas
de la batería.

Durante las primeras etapas del proyecto, el circuito podrá implementarse en
una placa de pruebas o en una placa perforada. Sin embargo, la implementación
final se realizará en una PCB diseñada específicamente para el sistema.

## Problema que resuelve

En entornos académicos y de laboratorio, el estudio y monitoreo de baterías
suele depender de cicladores comerciales, instrumentos independientes,
interfaces limitadas o procedimientos manuales para procesar los datos
obtenidos.

Esta situación puede generar las siguientes problemáticas:

- Los cicladores de baterías comerciales suelen tener un costo elevado.

- Algunos equipos solamente se encuentran disponibles en laboratorios
  específicos, lo que limita su acceso, movilidad y disponibilidad.

- Determinadas interfaces y herramientas de análisis requieren licencias
  costosas.

- Algunos sistemas comerciales ofrecen poca flexibilidad para configurar
  pruebas experimentales específicas.

- El uso de varios instrumentos independientes dificulta la integración de los
  procesos de carga, descarga, medición y almacenamiento de datos.

- El procesamiento manual de los datos aumenta el tiempo necesario para
  analizar el comportamiento de la batería y puede introducir errores.

El circuito propuesto busca ofrecer una alternativa de menor costo, orientada
al uso académico y experimental, que integre en un mismo sistema el control de
la corriente, la ejecución de los ciclos y la supervisión de las variables de
la batería.

## Contexto de uso

El sistema está diseñado para operar en entornos académicos, de investigación
o de laboratorio técnico, donde se realicen pruebas controladas de carga,
descarga y ciclado de baterías.

El circuito ciclador estará conectado a una computadora mediante un protocolo
de comunicación previamente definido. La computadora funcionará como interfaz
de supervisión y configuración, permitiendo establecer los parámetros de la
prueba, iniciar o detener el funcionamiento del sistema y observar las
variables eléctricas medidas.

El dispositivo deberá utilizarse bajo condiciones controladas y con una
batería compatible con las especificaciones eléctricas establecidas durante el
diseño.

En esta etapa, el proyecto no contempla aplicaciones industriales de alta
potencia ni su integración en sistemas críticos de producción. Su propósito
principal es facilitar la experimentación, el análisis y la validación del
comportamiento de baterías dentro de un laboratorio.

## Alcances

El proyecto contempla el diseño y la implementación de un circuito ciclador
con su respectivo sistema de control. Los principales alcances son los
siguientes:

- Diseñar el circuito eléctrico encargado de realizar los procesos de carga y
  descarga de la batería.

- Dimensionar los inductores, transistores, sensores, elementos de protección
  y demás componentes necesarios para el funcionamiento del sistema.

- Implementar un sistema de control robusto y eficiente que permita regular la
  corriente de la batería y corregir las perturbaciones que puedan presentarse
  durante la operación.

- Simular el comportamiento del circuito y del sistema de control antes de
  construir el prototipo físico.

- Diseñar una PCB capaz de soportar las corrientes y temperaturas esperadas
  durante múltiples ciclos de carga y descarga.

- Construir y ensamblar el prototipo físico mediante la soldadura de los
  componentes sobre la PCB.

- Implementar la comunicación entre el circuito y una computadora para
  configurar y supervisar las pruebas.

- Diseñar una carcasa que proteja la PCB y permita utilizar el dispositivo de
  forma segura dentro del laboratorio.

- Proporcionar protección ante pequeños golpes y ante las condiciones normales
  de manipulación del equipo.

- Obtener mediciones de las principales variables eléctricas de la batería,
  como corriente y tensión, durante los procesos de carga y descarga.

## Limitaciones del sistema

El sistema presenta las siguientes limitaciones:

- El diseño electrónico y el sistema de control se desarrollarán inicialmente
  para un modelo específico de batería.

- Aunque el circuito pueda probarse con baterías de características similares,
  no se puede garantizar su correcto funcionamiento fuera de las condiciones
  eléctricas para las cuales fue diseñado.

- El rango de tensión y corriente estará limitado por las características de
  los componentes seleccionados.

- La precisión de las mediciones dependerá de los sensores, convertidores y
  sistemas de adquisición utilizados.

- El comportamiento obtenido en la implementación física puede diferir de los
  resultados de simulación debido a pérdidas, tolerancias, ruido eléctrico,
  retardos y efectos térmicos.

- La calidad de las simulaciones dependerá de la precisión de los modelos de la
  batería y de los componentes electrónicos disponibles.

- Pueden presentarse problemas de convergencia o limitaciones durante las
  simulaciones realizadas en PLECS, PSpice o LTspice.

- La fabricación del prototipo dependerá de los plazos de producción y entrega
  de la PCB.

- Algunos componentes deberán adquirirse en el extranjero, lo que puede
  generar retrasos y costos adicionales.

- El prototipo no estará diseñado para aplicaciones industriales de alta
  potencia ni para sistemas críticos.

- El sistema deberá utilizarse dentro de un entorno controlado y bajo la
  supervisión de personal con conocimientos básicos de electrónica y manejo de
  baterías.



## [Simulación](PLECS/)

En esta sección se encuentra el código utilizado para la implementación en PLECS y así poder utilizar un C2000, en esta misma también se muestran las simualciones.

## [PCB](PCB/)

En esta sección se encuentran los archivos utilizados para la creación de la PCB.
