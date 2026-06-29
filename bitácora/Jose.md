# Bitácora del Proyecto Eléctrico

Jose Manuel Solis

Sistema Electrónico para Ciclado y Evaluación de Vida Útil de Baterías


## Objetivos

### Objetivo General

Diseñar e implementar un sistema integral de ciclado de baterías basado en una tarjeta de circuito impreso y controlado mediante un microcontrolador ESP32, capaz de realizar procesos programables de carga y descarga, monitorear y registrar variables eléctricas relevantes para la estimación del estado de la batería y su vida útil, e integrarse con una interfaz de usuario en computadora que permita la selección del tipo de batería, configuración de parámetros de operación y análisis del comportamiento a lo largo de múltiples ciclos.

### Objetivos Específicos

- Diseñar una placa de circuito impreso (PCB) para el ciclador de baterías, mediante la construcción y simulación de circuitos eléctricos en KiCad y PLECS, con el fin de implementar un sistema capaz de realizar procesos programables de carga y descarga, así como medir variables eléctricas relevantes del sistema.
- Diseñar un sistema de comunicación serial entre la PCB del ciclador de baterías y la interfaz gráfica, utilizando un protocolo de comunicación y un microcontrolador, con el fin de establecer un canal de control entre la interfaz de usuario y el hardware del sistema (PCB).
- Diseñar una interfaz gráfica de usuario mediante el lenguaje C++ y la librería WxWidgets, con el fin de permitir la configuración de parámetros de operación, la selección del tipo de batería y el monitoreo de variables durante los ciclos de carga y descarga.

## Entregables

- Repositorio de GitHub.
- PCB funcional.
- Manual de instalación de interfaz gráfica.
- Prototipos y sistema final implementado.

> Nota: En este proyecto se forma parte del subgrupo encargado del diseño electrónico, el diseño del sistema de control y el hardware del sistema PCB.

## 1. Semana 1

### 1.1. Reunión con Mauricio

**Fecha:** 9 de Marzo, 2026

**Horas:** De 10 a.m. a 11:30 a.m. (1 hora y 30 minutos)

#### Objetivos

- Presentarle el proyecto al profesor Mauricio Espinoza, comentar la división que se va a implementar, las partes y el prototipo final que se quiere entregar.

#### Actividades y Resultados

- Se realizó una reunión en Zoom con el profesor Mauricio para presentarle la propuesta del proyecto que se desea realizar.

#### Comentarios

- El profesor acepta la propuesta y brinda ideas de lo que él espera como prototipo final del proyecto.

### 1.2. Reunión de diseño conceptual del prototipo con los compañeros

**Fecha:** 9 de marzo, 2026

**Horas:** De 7 p.m. a 10 p.m. (3 horas)

#### Objetivos

- Presentar y discutir las ideas del grupo, así como desarrollar un esquema preliminar del modelo a implementar.

#### Actividades y Resultados

- Se expuso la idea de hacer un ciclador de baterías el cual se pueda conectar a la computadora y obtener datos de la batería.
- Se propuso dividir el proyecto en 3 secciones: la PCB y el sistema de control, el protocolo de comunicación y la interfaz gráfica.
- Se proponen 3 metas claras: que se pueda ciclar la batería, que se pueda cargar o descargar completamente y que se pueda cargar o descargar de un porcentaje a otro; en todos los casos obteniendo datos para calcular la vida útil y la capacidad de carga de la batería.
- Se hizo un dibujo para organizar las ideas de cómo debe funcionar el prototipo.

#### Comentarios

- Queda pendiente definir el microcontrolador. Las dos opciones que se tienen son el C2000 de Texas Instruments o el ESP32.

### 1.3. Diseño del circuito de ciclado

**Fecha:** 10 de Marzo, 2026

**Horas:** De 1 p.m. a 5 p.m. (4 horas)

#### Objetivos

- Diseñar el circuito y el lazo de control que se encargue de ciclar la batería.

#### Actividades y Resultados

- Se diseñó un circuito basado en una etapa de salida de cargas con el fin de que los transistores se encarguen de controlar la corriente de la batería, la cual se cataloga como la perturbación al cargar y descargar la batería.
- Se plasma una idea de un lazo de control de forma que por medio de los amplificadores operacionales se pueda encender y apagar los transistores.

#### Comentarios

- Falta diseñar bien el lazo de control. Se tiene pensado un lazo de control prealimentado ya que se puede medir la perturbación.

## 2. Semana 2

### 2.1. Reunión con Mauricio para discutir el proyecto

**Fecha:** 16 de Marzo, 2026

**Horas:** De 9 a.m. a 11 a.m. (2 horas)

#### Objetivos

- Comentarle al profesor Mauricio el diseño conceptual que se tiene pensado y proponer utilizar el C2000 como microcontrolador.

#### Actividades y Resultados

- Se realizó una reunión con Mauricio en la que se propuso el prototipo planteado.
- El profesor Mauricio nos ayudó a definir las ideas y los entregables del proyecto.
- El profesor nos propuso utilizar el ESP32 por su versatilidad con la comunicación.

#### Comentarios

- El profesor nos enseñó diferentes sensores que podemos tomar en cuenta y aprobó el prototipo presentado.

### 2.2. Simulación del circuito

**Fecha:** 17 de Marzo, 2026

**Horas:** De 1 p.m. a 5 p.m. (4 horas)

#### Objetivos

- Verificar el diseño del circuito pensado anteriormente.
- Simular el circuito y verificar que puede cargar y descargar la batería con la topología utilizada.

#### Actividades y Resultados

- Se definió un lazo de control prealimentado-realimentado, ya que la corriente, que es la perturbación, se puede medir con sensores de corriente. Además, se puede agregar un controlador PI para que se siga mejor la referencia.
- Se simuló el circuito y el lazo de control en PSpice, generando la gráfica del comportamiento de la batería. En la gráfica se puede confirmar que el circuito se comporta como se esperaba, cargando y descargando la batería según se indique en la referencia.

#### Comentarios

- Falta hacer los cálculos de la señal de control para verificar que el circuito sí cumple con las ecuaciones de carga y descarga.

### 2.3. Cálculo de la acción de control

**Fecha:** 11 de Marzo, 2026

**Horas:** De 3 p.m. a 4 p.m. (1 hora)

#### Objetivos

- Calcular la acción de control de la malla de carga y descarga.

#### Actividades y Resultados

- Se hicieron los cálculos correspondientes para identificar la acción de control de cada malla.
- La acción de control dio como se esperaba: el circuito de carga positivo y el de descarga negativo.

#### Comentarios

- Se aseguró que el circuito funciona, por lo que lo próximo es buscar los componentes y diseñar la PCB.

### 2.4. Inicio del diseño de la PCB

**Fecha:** 13 de Marzo, 2026

**Horas:** De 4 p.m. a 7 p.m. (3 horas)

#### Objetivos

- Diseñar el esquemático inicial de la PCB en KiCad para mostrarle al profesor.
- Hacer un esquemático en el que se contemplen todas las entradas y salidas de forma que solo se tengan que colocar los dispositivos como el microcontrolador o los sensores.

#### Actividades y Resultados

- Se hizo todo el esquemático de la PCB contemplando las entradas y salidas.
- Se definieron algunos componentes como resistencias, capacitores e inductores que se definieron previamente en la simulación.

#### Comentarios

- Queda pendiente enseñársela al profesor para la aprobación.
- Queda pendiente definir bien los pines del ESP32 con el equipo de comunicación.
- Falta definir los componentes de la PCB.

## 3. Semana 3

### 3.1. Reunión con el profesor Mauricio

**Fecha:** 23 de Marzo, 2026

**Horas:** De 10 a.m. a 12 m.d. (2 horas)

#### Objetivos

- Resolver dudas sobre el circuito de carga.
- Resolver dudas sobre el sistema de control.
- Mostrarle la PCB para verificar si falta algo o en qué se puede mejorar.

#### Actividades y Resultados

- Se aclaró la duda de la alimentación del circuito de carga, ya que este tiene que tener la fuente de tensión que carga la batería.
- Se resolvió la duda de cómo el sistema de control se encarga de suprimir la perturbación. Además, se aclaró cómo funciona la referencia en las simulaciones, ya que al ser una senoide en la simulación causaba una confusión sobre si había que generar una senoide con el microcontrolador.
- Se mostró la PCB y Mauricio nos recomendó hacer unos cambios, entre ellos utilizar cables directos desde la batería hasta el sensor de tensión solo para medir esta, con el fin de evitar las resistencias parásitas. Otra recomendación fue integrar todos los componentes en una sola PCB, por lo que los convertidores DC-DC se deben acomodar ahí también.
- Se nos recomendó investigar sobre baterías para definir la batería que se va utilizar o las que se van utilizar, con el fin de definir las corrientes y las tensiones del microcontrolador.
- Se recomendó hacer el modelo de la caja en donde se va a meter todo para ordenar la PCB con la posición de los conectores y tener el modelo de cómo conectar la batería.

#### Comentarios

- Queda pendiente diseñar la caja del prototipo antes de seguir con la PCB.
- Se le encarga a Eduardo Rojas hacer la investigación de las baterías para definir el modelo y con eso hacer los cálculos de carga y descarga.
- Se debe investigar para utilizar 4 cables en la batería.
- Queda pendiente investigar sobre sensores LEM y el de tensión, buscar opciones accesibles para tener de dónde elegir cuando se tengan los datos de la batería.

### 3.2. Búsqueda de los componentes electrónicos

**Fecha:** 24 de Marzo, 2026

**Horas:** De 3 p.m. a 7 p.m. (4 horas)

#### Objetivos

- Hacer una lista de los componentes a utilizar, en donde se tenga la hoja de datos, el link de compra y el precio.
- Definir el microcontrolador que se va a utilizar.

#### Actividades y Resultados

- Se buscaron y compararon los componentes que se van a utilizar preliminarmente.
- Se buscaron las características de los microcontroladores C2000 y ESP32.
- Se generó un documento en el que se tienen el componente, la hoja de datos, el precio y el link para comprarlo.
- Se escogió el microcontrolador ESP32 debido a sus características para comunicación. Si bien no es el más dotado para la parte de control, tiene lo que se requiere. Además, cuenta con muchas herramientas para comunicación.

#### Comentarios

- Al utilizar diferentes baterías se requieren diferentes tensiones controladas por medio de la computadora. Por eso se requiere un convertidor DC-DC variable, que se va a hacer comprando un convertidor DC-DC normal y colocándole un trimmer digital en el trimmer que ya viene.
- Esta es una lista preliminar y está sujeta a modificaciones si se requieren más componentes.

### 3.3. Continuación del diseño de la PCB

**Fecha:** 27 de Marzo, 2026

**Horas:** De 3 p.m. a 6 p.m. (3 horas)

#### Objetivos

- Diseñar el modelo 3D de la PCB en KiCad.
- Hacer un modelo 3D de la PCB en el que se puedan colocar sobre pines tipo jack el ESP32 y sensores.
- Hacer una PCB de menos de 10 cm x 10 cm para ahorrar en los costos de fabricación.

#### Actividades y Resultados

- Se hizo el modelo 3D de la PCB contemplando las entradas y salidas.
- Se tomaron en cuenta las medidas del ESP32 y los sensores de Arduino, por el momento, para solo tener que colocarlos sobre la PCB.

#### Comentarios

- Queda pendiente enseñársela al profesor para la aprobación.
- Queda pendiente definir los sensores.
- Queda pendiente definir si colocar o no los convertidores en la PCB.

## 4. Semana 4 (Semana Santa)

### 4.1. Investigación de sensores

**Fecha:** 31 de Marzo, 2026

**Horas:** De 10 a.m. a 12 m.d. (2 horas)

#### Objetivos

- Investigar sobre sensores LEM de corriente y el sensor de tensión.

#### Actividades y Resultados

- Se hace una búsqueda de sensores tipo LEM y sensores de Arduino, los cuales ya se tenían presupuestados.
- Se ven unos sensores de corriente de Arduino en el laboratorio y se guardan como posibles opciones.
- Se estudian los sensores de Arduino, ya que se ocupa que el sensor no genere mucho ruido.

#### Comentarios

- Se requiere la información de la batería para poder escoger mejor el sensor. Sin embargo, los sensores de Arduino se tienen como principal opción porque ya se tienen 2 de ellos.

## 5. Semana 5

### 5.1. Reunión con Mauricio

**Fecha:** 7 de Abril, 2026

**Horas:** De 2 p.m. a 3 p.m. (1 hora)

#### Objetivos

- Conversar sobre los avances al proyecto y planificar avances.

#### Actividades y Resultados

- Se hace una reunión en la que se conversó sobre las entradas y salidas de la PCB, el sistema de control y la batería.

#### Comentarios

- La batería sigue sin definirse, pero se tienen opciones disponibles que se pueden utilizar.
- Se comentó que el convertidor DC/DC no va a dar suficiente variación como para contemplar todos los valores de tensión de todas las opciones de batería.

### 5.2. Búsqueda de convertidor DC/DC

**Fecha:** 10 de Abril, 2026

**Horas:** De 1 p.m. a 4 p.m. (3 horas)

#### Objetivos

- Buscar el convertidor DC/DC que se tenía y verificar que dé el rango de variación de tensión para las diferentes baterías.

#### Actividades y Resultados

- Se buscó la hoja de datos del convertidor DC/DC que se había seleccionado para buscar el rango de variación.
- Al ver la hoja de datos se determinó que no tiene tanto rango de variación y que solo se pueden seleccionar valores fijos de tensión dependiendo de lo que se le meta.

#### Comentarios

- Se concluye que no se puede utilizar el convertidor DC/DC debido a que no va a funcionar. Queda pendiente decidir como equipo qué batería utilizar y posteriormente modificar la electrónica para poder utilizar diferentes baterías.

## 6. Semana 6

### 6.1. Reunión para definir la batería y conversar los avances internos

**Fecha:** 14 de Abril, 2026

**Horas:** De 1 p.m. a 3 p.m. (2 horas)

#### Objetivos

- Conversar con los compañeros de grupo sobre la batería que se va a utilizar en el prototipo inicial.

#### Actividades y Resultados

- Se hace una reunión en la universidad con los 5 miembros del grupo en la que se definió la batería de 12 V que se tiene en el Lab-Ces.
- Además, se habla de los avances en el sistema de comunicación, la PCB y la interfaz gráfica.

#### Comentarios

- Se definió la batería y se deja como tarea buscar el perfil de carga de la batería y calcular los datos para la carga y descarga para poder programar el ESP32 con los datos correctos.

### 6.2. Modificación de la PCB

**Fecha:** 17 de Abril, 2026

**Horas:** De 1 p.m. a 5 p.m. (4 horas)

#### Objetivos

- Modificar la PCB para utilizar menos área y colocar los sensores fuera de la PCB, con el objetivo de ahorrar espacio y no tener que modificar las terminales de los sensores.

#### Actividades y Resultados

- Se modifica el diseño de la PCB para reducir el espacio, colocando solo las terminales de alimentación y señal de los sensores.

#### Comentarios

- Se dejan las terminales del convertidor DC/DC, del trimmer digital y las del trimmer digital para modificarlo en un futuro si se requiere otra tensión.
- No se terminó el diseño, queda pendiente mostrar los modelos hechos al grupo y hacer las rutas en la placa.

## 7. Semana 7

### 7.1. Hacer una lista de los componentes propios de la PCB

**Fecha:** 20 de Abril, 2026

**Horas:** De 3 p.m. a 6 p.m. (3 horas)

#### Objetivos

- Hacer la lista de los componentes propios de la PCB que no se habían incluido en la lista de los materiales.

#### Actividades y Resultados

- Se toma el prototipo de PCB y se cuentan los conectores pin socket hembra, los conectores banana, los conectores tipo bloque y otros componentes.
- En una pequeña conversación con Mauricio se habla de retrasos en los pedidos de componentes y PCB, por lo que se debe pensar en soluciones alternas.

#### Comentarios

- Al tener problemas con los componentes, se deja como tarea buscar los componentes en tiendas físicas o virtuales en Costa Rica y comparar los presupuestos.
- Se deja como tarea buscar opciones diferentes a una PCB para hacer un prototipo con el que se puedan hacer pruebas.

### 7.2. Investigar sobre la soldadura en una placa perforada

**Fecha:** 22 de Abril, 2026

**Horas:** De 3 p.m. a 5 p.m. (2 horas)

#### Objetivos

- Hacer una búsqueda en YouTube y repositorios en internet donde se explica cómo soldar en una placa perforada de prototipos y cómo se utiliza.

#### Actividades y Resultados

- Se ven videos sobre la soldadura en una placa perforada y se buscan en tiendas de electrónica las placas disponibles y sus precios.

#### Comentarios

- Al ver los videos y analizarlo queda como opción disponible, pero queda comentarlo con el profesor.

## 8. Semana 8

### 8.1. Selección de la PCB dentro de los modelos propuestos

**Fecha:** 1 de Mayo, 2026

**Horas:** De 1 p.m. a 3 p.m. (2 horas)

#### Objetivos

- Seleccionar la mejor opción de PCB para presentarle al profesor Mauricio.

#### Actividades y Resultados

- Se piensa en una futura carcasa, la forma de acomodar los sensores externos y los cables que se deben conectar a la PCB, por lo que se elige la opción en la que los sensores de corriente y tensión se conectan por debajo para ahorrar espacio y que se puedan atornillar a una carcasa.

#### Comentarios

- Queda pendiente mostrarle a Mauricio la PCB para ver si le parece bien.

### 8.2. Búsqueda de los conectores que faltaban para la PCB

**Fecha:** 2 de Mayo, 2026

**Horas:** De 1 p.m. a 3 p.m. (2 horas)

#### Objetivos

- Hacer una búsqueda de los conectores tipo socket hembra que faltan y otros conectores que no se tienen en la lista.

#### Actividades y Resultados

- Se hace una búsqueda en la página de componentes electrónicos y CRcibernética, pero solo se encuentra el conector del cargador y el conector tipo bloque en el que se atornillan los cables. Los conectores tipo socket no se encontraron.

#### Comentarios

- Queda pendiente adjuntarlos en la lista, ya que solo se encontraron en Amazon.

## 9. Semana 9

### 9.1. Reunión con Mauricio para resolver dudas con la PCB

**Fecha:** 6 de Mayo, 2026

**Horas:** De 12 p.m. a 1 p.m. (1 hora)

#### Objetivos

- Conversar con el profesor sobre la PCB y una duda que surgió sobre el circuito de carga y descarga.

#### Actividades y Resultados

- Se hizo una reunión presencial con el profesor para ver el diseño de la PCB escogida. Se determinó que por tipo de conexión de la PCB el circuito de polarización inversa no es necesario, por lo que se va a quitar y se va a reordenar para poner los inductores aparte porque pueden generar ruido.
- Se hizo la consulta sobre el circuito, ya que al simularlo no se tenía muy claro si cargaba bien la batería. El profesor indicó que se debe revisar la configuración del amplificador operacional, ya que no está amplificando, y también sacar la función de transferencia desde la entrada manipulable hasta la variable controlada.
- Al tener problemas con la PCB, el profesor dijo que en lugar de hacer un prototipo en placa perforada prefiere que utilicemos un modelo de pruebas tipo hardware in the loop, con la función de transferencia de carga y descarga implementada en un microcontrolador para probar el programa y la comunicación.

#### Comentarios

- Se va a hacer el cálculo de las funciones de transferencia y la condición de carga y descarga.
- Se van a implementar las pruebas mediante un controlador C2000 para poder programarlo por medio de PLECS y evitar problemas de comunicación o programación.

### 9.2. Revisión de la simulación para revisar el rango de tensión del transistor y la amplificación

**Fecha:** 8 de Mayo, 2026

**Horas:** De 8 p.m. a 11 p.m. (3 horas)

#### Objetivos

- Revisar la simulación de PSpice que se tenía para ver si con un solo amplificador operacional se lograba amplificar lo suficiente el rango que da el ESP32.

#### Actividades y Resultados

- Se vio que con la configuración que se tenía no amplificaba nada, solo se tenía una caída de tensión.
- Se buscó la configuración usual de amplificación de los amplificadores operacionales y se realizaron los cálculos de las resistencias para lograr una ganancia en la que se tenga más tensión que el valor nominal de la batería.

#### Comentarios

- Queda pendiente probar la simulación utilizando un barrido para comprobar que sí amplifique la tensión.
- Esa simulación es preferible hacerla en un esquemático diferente para no tocar el módulo de control que se tiene en PSpice.

## 10. Semana 10

### 10.1. Simulación del circuito amplificador

**Fecha:** 13 de Mayo, 2026

**Horas:** De 2 p.m. a 6 p.m. (4 horas)

#### Objetivos

- Hacer el circuito de carga en LTspice y simularlo utilizando un barrido DC para verificar la tensión.

#### Actividades y Resultados

- Se construyó la etapa de carga del circuito utilizando una fuente de 15 V para cargar y una fuente de 12 V como batería. También se configuró el amplificador operacional para que tenga una configuración de amplificación.
- Al inicio se tuvo problemas para formar la configuración de amplificación, ya que se colocaban mal los cables y se conectaban cables a nodos que no eran. Sin embargo, luego de un tiempo se logró obtener la configuración y probar la amplificación, que llegó aproximadamente a 14 V.
- La simulación utilizada es un DC Sweep de 0 a 3.3 V en la terminal positiva del amplificador operacional, que es donde se conecta el ESP32. Este valor está en el eje X y en el eje Y se tiene el valor de amplificación.

#### Comentarios

- Como se sabe que la amplificación sí llega a un nivel alto con el circuito implementado, el próximo paso es sacar la función de transferencia para implementarla en el C2000 y comenzar las pruebas con hardware in the loop.

## 11. Semana 11

### 11.1. Cálculo de la función de transferencia

**Fecha:** 22 de Mayo, 2026

**Horas:** De 3 p.m. a 6 p.m. (3 horas)

#### Objetivos

- Calcular la función de transferencia de carga y descarga para implementarla en un microcontrolador.

#### Actividades y Resultados

- Se tomó el circuito de carga y se realizó la LTK en la malla para obtener la suma de las tensiones. Al sustituir con la ecuación del inductor y sacar la corriente se puede identificar la entrada y salida del circuito y con esto sacar la función de transferencia.

#### Comentarios

- Al obtener esta expresión se le preguntó al profesor cómo interpretarla. Él indicó que se debe poner tensión en las tensiones del denominador que se igualan a \(V_{eq}(s)\), ya que de aquí sale la condición que debe cumplir la tensión del transistor para que el circuito cargue la batería.
- En el caso del circuito de descarga se utiliza la misma expresión pero con signo negativo, ya que los circuitos son iguales y ya se había calculado la acción de control que determinó que el signo del circuito de descarga es negativo.

## 12. Semana 12

### 12.1. Simulación del circuito y sistema de control en PSpice

**Fecha:** 25 de Mayo, 2026

**Horas:** De 5 p.m. a 8 p.m. (3 horas)

#### Objetivos

- Simular el circuito actualizado con el sistema de control implementado en PSpice.

#### Actividades y Resultados

- Se modificó el circuito que ya se tenía, se calcularon de nuevo las ganancias del controlador y se simuló. Sin embargo, la simulación comenzó a dar error por convergencia en las corrientes.
- Se simplificó el circuito quitando la etapa de descarga y se volvió a simular, pero seguía dando problemas.

#### Comentarios

- La simulación no funcionó. Al cambiar la ganancia de la señal de entrada algo pasa dentro del circuito, puesto que está generando el error en el simulador.

### 12.2. Reunión con Mauricio

**Fecha:** 26 de Mayo, 2026

**Horas:** De 3 p.m. a 6 p.m. (3 horas)

#### Objetivos

- Hablar con el profesor Mauricio para preguntarle por la simulación.

#### Actividades y Resultados

- Se habló con el profesor y se estuvo tratando de simular el circuito, pero no daba resultado.
- Se encontraron inconsistencias en el circuito y en el sistema de control, como las ganancias y los modelos, por lo que el profesor indicó que se tenía que corregir.

#### Comentarios

- Queda pendiente corregir las inconsistencias en el circuito.

### 12.3. Simulación del circuito y sistema de control en PSpice parte II

**Fecha:** 28 de Mayo, 2026

**Horas:** De 5 p.m. a 8 p.m. (3 horas)

#### Objetivos

- Revisar qué está pasando en el circuito, puesto que no funciona la simulación.

#### Actividades y Resultados

- Se cambió el amplificador operacional, ya que el que se tenía no era el que se había pensado al inicio. Entonces, se importó el modelo de Texas Instruments y se simuló de nuevo.
- Al cambiar el amplificador ya corrió la simulación, pero el resultado no es muy bueno. El sistema de control no está funcionando como debería.

#### Comentarios

- Se corrigió el circuito, pero no se obtuvo el resultado esperado en la simulación, por lo que queda pendiente hablar con el profesor para buscar una solución.
- Queda pendiente hacer el cambio en la PCB, ya que se tiene que añadir la etapa de amplificación.
- El día 29 de mayo se habló rápidamente con el profesor y nos indicó que simulemos en PLECS como última opción, ya que en PSpice puede que el controlador PI en el fondo no esté realizando esa función por una cuestión matemática del simulador.

## 13. Semana 13

### 13.1. Simulación del circuito en PLECS

**Fecha:** 1 de Junio, 2026

**Horas:** De 4 p.m. a 8 p.m. (4 horas)

#### Objetivos

- Montar el circuito y el sistema de control en el simulador PLECS.

#### Actividades y Resultados

- Se construyó el circuito siguiendo las instrucciones del profesor sobre la topología del circuito y el sistema de control y se simuló en PLECS.
- El simulador dio un resultado que no se esperaba, ya que no se puede controlar la corriente. Las curvas se asemejan a lo que se quiere, pero hay picos de corriente que indican que hay algo mal en la simulación.

#### Comentarios

- Se nota que PLECS funciona mejor que PSpice, pero todavía está costando obtener la simulación bien.

### 13.2. Consulta al profesor Mauricio sobre la simulación en PLECS

**Fecha:** 2 de Junio, 2026

**Horas:** De 4 p.m. a 6 p.m. (2 horas)

#### Objetivos

- Aclarar las dudas sobre el circuito y lograr controlar la corriente con la simulación.
- Obtener un circuito para hacer las pruebas de hardware in the loop.

#### Actividades y Resultados

- Se revisó el circuito y con ayuda del profesor se identificó una fuente que no debía ir en la simulación. Al simularlo de nuevo se logró controlar la corriente bastante bien con el controlador PI.
- Se identifica que la resistencia parásita del inductor provoca problemas con la corriente del circuito aunque sea un valor muy bajo, por lo que se debe revisar la hoja de datos del inductor para obtener esa resistencia y simular con el valor correcto.

#### Comentarios

- Al revisar el inductor nos enteramos de que la corriente máxima está por debajo de la corriente que se va a utilizar, por lo que se debe buscar otro y tener especial cuidado con la corriente.
- Se va a simular con la resistencia nominal del inductor, pero a la hora de armar el circuito en placa perforada o en PCB se debe medir la resistencia del inductor con el método de 4 cables, para obtener el valor más preciso y configurar así el software del sistema.

### 13.3. Reunión virtual con los compañeros

**Fecha:** 3 de Junio, 2026

**Horas:** De 2 p.m. a 5 p.m. (3 horas)

#### Objetivos

- Comentar el avance de cada equipo de trabajo con el fin de conocer qué hace falta y coordinar las tareas pendientes.
- Ponerse de acuerdo para crear la presentación que se debe exponer en el curso de Proyecto Eléctrico el martes 9 de Junio.

#### Actividades y Resultados

- Se realizó una reunión virtual con los compañeros para ponerse al tanto del avance de cada parte del proyecto y lo que hace falta completar.
- Se consultó si algún equipo ocupaba ayuda de los demás debido a atrasos o dificultades en alguna parte del proyecto.
- Se comentó que el viernes 5 de junio del 2026 se va a realizar una reunión presencial para hacer la primera prueba hardware in the loop con el C2000 y el ESP32.

#### Comentarios

- Queda pendiente preparar la presentación del curso y realizar la primera prueba presencial.

### 13.4. Prueba hardware in the loop

**Fecha:** 5 de Junio, 2026

**Horas:** De 2 p.m. a 6 p.m. (4 horas)

#### Objetivos

- Hacer la primera prueba hardware in the loop para verificar el sistema de comunicación y la interfaz gráfica.

#### Actividades y Resultados

- Se conectó por primera vez el ESP32 con el Delfino C2000. Al C2000 se le conecta PLECS con el circuito que se simuló y al ESP32 se le configuran los datos del programa.
- Al conectarlos y cargar el programa tanto en el ESP32 como en el C2000, todo se ejecutó muy bien. Pero al momento de correrlos a la vez, con el fin de que el C2000 funcione como la planta, se comenzaron a recibir datos que no se esperaban, por lo que se debe revisar dónde está el error.

#### Comentarios

- La primera parte de la prueba funcionó correctamente, pero los datos que da la prueba son extraños, por lo que se debe revisar dónde está el fallo. Se sospecha que es en el C2000.

## 14. Semana 14

### 14.1. Pruebas hardware in the loop

**Fecha:** 9 de Junio, 2026

**Horas:** De 1 p.m. a 4 p.m. (3 horas)

#### Objetivos

- Probar el sistema de control del ESP32 con el circuito de carga de PLECS.

#### Actividades y Resultados

- Se conectó el ESP32 al C2000 con la configuración de carga, se ejecutaron los archivos y se inició la prueba.
- Se le preguntó al profesor Mauricio cómo se podían ver en tiempo real las señales de PLECS. Él mostró la forma de hacerlo con external mode y de esta forma logramos ver que el circuito sí estaba cargando.
- La interfaz gráfica mostraba ciertos problemas para conexión y para mostrar los datos.

#### Comentarios

- Se acuerda con el profesor utilizar otro C2000 para simular la batería, ya que una fuente de tensión define la tensión entre sus terminales, entonces no se puede ver un cambio en esta.
- Se anotaron las fallas de comunicación, del circuito y de la interfaz para corregirlas y probar de nuevo.

### 14.2. Correcciones en el diseño de la PCB

**Fecha:** 12 de Junio, 2026

**Horas:** De 3 p.m. a 8 p.m. (5 horas)

#### Objetivos

- Hacer las correcciones que el profesor Mauricio indicó en el prototipo de la PCB.

#### Actividades y Resultados

- Se elimina el circuito de protección de polarización inversa, ya que este no es necesario por el tipo de conexión de la alimentación.
- Se añaden los amplificadores operacionales para amplificar la señal de carga y descarga del ESP32.
- Se hace el cambio de los inductores por el modelo adecuado para la corriente que va a manejar la batería.
- Se importan los modelos 3D de los sensores y del ESP32 para mostrar la conexión de todas las partes.

#### Comentarios

- Se debe presentar la PCB al profesor para ver si ya el prototipo cumple con lo necesario para la entrega.
- Se debe actualizar el presupuesto y la lista de componentes, ya que se deben agregar amplificadores operacionales y se eliminan los componentes del circuito de protección de polarización inversa.

## 15. Semana 15

### 15.1. Implementación del modelo de batería en el hardware in the loop

**Fecha:** 18 de Junio, 2026

**Horas:** De 3 p.m. a 8 p.m. (5 horas)

#### Objetivos

- Probar el sistema implementando un modelo de batería más real para medir la tensión.

#### Actividades y Resultados

- Se conectó el ESP32 al C2000 con el circuito y a esto se le conecta el C2000 con el modelo de la batería.
- Se tienen diferentes problemas ya que no se puede visualizar el cambio de la tensión en la batería.
- Se deben hacer ajustes en la referencia de carga y el circuito, ya que el modelo de batería es de menos tensión. Sin embargo, sigue sin funcionar.
- Para este punto el circuito de PLECS ya está completo con carga y descarga, por lo que se tiene modelado el sistema completo.

#### Comentarios

- Se propone revisar y entender mejor el modelo de la batería, ya que es un poco complejo.
- Se encuentran más puntos para corregir en la aplicación, por lo que se anotan para corrección.
- Se le preguntó al profesor Mauricio sobre la PCB y mencionó que es mejor dividir la PCB en una sección de potencia y otra de electrónica, acortando las pistas de los sensores de corriente.

### 15.2. Se recopiló el avance del proyecto en el repositorio de GitHub

**Fecha:** 19 de Junio, 2026

**Horas:** De 4 p.m. a 9 p.m. (5 horas)

#### Objetivos

- Tomar las simulaciones, PCB y documentación del proyecto y subirla al repositorio junto con su respectiva explicación.

#### Actividades y Resultados

- Se subió la propuesta de diseño utilizada, las simulaciones de PLECS para comprobar que se podía controlar, los circuitos para hardware in the loop y los archivos de la PCB.
- Además, se organizó de forma que cada README esté ordenado y con las explicaciones generales de cada sección correspondiente al ciclador.

#### Comentarios

- Queda pendiente agregar los archivos de fabricación de la PCB debido a que se le debe hacer una modificación al diseño.
- Queda pendiente actualizar la última versión de la simulación hardware in the loop.

### 15.3. Corrección final de la PCB

**Fecha:** 10 de Junio, 2026

**Horas:** De 10 a.m. a 2 p.m. (4 horas)

#### Objetivos

- Corregir la PCB como el profesor indicó, generar las pistas y rellenar las capas. Además, insertar los textos que indican para qué es cada componente, incluyendo el indicador de la polaridad del conector de la alimentación.

#### Actividades y Resultados

- Se reordena en el editor 3D los componentes de la PCB considerando un sector de potencia y un sector de electrónica.
- Se conectan las pistas de la PCB considerando un ancho de pista de 1.5 mm para las secciones que manejan corrientes altas y un ancho de 0.6 mm para las pistas que conectan componentes electrónicos de baja potencia.
- Se utilizó un diseño de 4 capas con rutas solo en las capas exteriores que corresponden a la tierra.

#### Comentarios

- Con estas modificaciones queda terminada la PCB, por lo que se sube la actualización al repositorio de GitHub, su esquemático y su diseño 3D, junto con los archivos de modelos, símbolos y huellas que se utilizaron.

## 16. Semana 16

### 16.1. Implementación del modelo de batería en el hardware in the loop

**Fecha:** 23 de Junio, 2026

**Horas:** De 3 p.m. a 7 p.m. (4 horas)

#### Objetivos

- Hacer pruebas con la batería para comprobar el funcionamiento.

#### Actividades y Resultados

- Se conectó el ESP32 al C2000 con el circuito y a esto se le conecta el C2000 con el modelo de la batería.
- Se muestra un comportamiento extraño en la aplicación. En el PLECS del circuito se ven las señales donde el ESP32 indica carga y descarga, pero la corriente tiene valores muy altos y al mismo tiempo, por lo que se le atribuye a un fallo en la conexión de la batería.

#### Comentarios

- Se debe revisar la batería, que es la que está generando problemas. Además, se debe revisar el control PI en el código del microcontrolador.

### 16.2. Continuación de las pruebas

**Fecha:** 25 de Junio, 2026

**Horas:** De 3 p.m. a 8 p.m. (5 horas)

#### Objetivos

- Hacer la última prueba utilizando un modelo de batería con otro controlador para verificar que funciona.
- Definir si seguir probando la batería o continuar modelando la batería en el circuito que ya se tiene.

#### Actividades y Resultados

- Se conecta la corriente y la tensión de forma diferente, ya que se tenía la tensión directo al ESP32 como una medición. Sin embargo, el circuito la ocupa, entonces se realizaron cambios en los pines y se configuró.
- El resultado mejoró un poco porque en el circuito y en la aplicación se podía ver un comportamiento de carga y descarga, pero la batería no mostraba nada.

#### Comentarios

- Se decide descartar la idea de usar una batería con otro C2000 debido a los problemas generados y a lo complejo del modelo de la batería.

### 16.3. Continuación de las pruebas

**Fecha:** 26 de Junio, 2026

**Horas:** De 10 a.m. a 8 p.m. (10 horas)

#### Objetivos

- Hacer las pruebas necesarias para obtener el resultado de simulación que se necesita.

#### Actividades y Resultados

- Se empezó por colocar una resistencia en serie con un capacitor para simular el comportamiento de la batería.
- Luego se construyó el modelo en el C2000 y se conectó al ESP32 para iniciar las pruebas.
- Al inicio dio bastantes problemas, ya que el controlador no estaba funcionando, haciendo que la corriente tuviera un comportamiento extraño.
- Luego de ciertas modificaciones en el código del controlador PI se logró controlar la corriente y, al ejecutar el ciclado, se logró obtener el resultado esperado, donde la batería se carga y descarga controlando la corriente y viendo la tensión final de esta.

#### Comentarios

- Se dan como finalizadas las pruebas de funcionamiento de hardware in the loop, el controlador y la aplicación.|



