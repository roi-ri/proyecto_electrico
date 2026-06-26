
# PCB del ciclador de baterías

## Descripción general

En este directorio se encuentran los archivos de diseño de la PCB utilizada
para implementar el circuito ciclador de baterías.

La placa integra las etapas de carga y descarga, el acondicionamiento de las
señales de control, las conexiones para los sensores, el ESP32 y las terminales
necesarias para conectar la alimentación y la batería.

La PCB fue diseñada en KiCad y utiliza cuatro capas de cobre:

- `F.Cu`: capa superior.
- `In1.Cu`: primera capa interna.
- `In2.Cu`: segunda capa interna.
- `B.Cu`: capa inferior.

Los archivos principales del proyecto son:

- `Proyecto.kicad_pro`: archivo principal del proyecto de KiCad.
- `Proyecto.kicad_sch`: esquemático eléctrico.
- `Proyecto.kicad_pcb`: diseño físico de la PCB.
- `symbols/`: símbolos personalizados.
- `footprints/`: huellas personalizadas.
- `3dmodels/`: modelos tridimensionales utilizados.
- `Imagenes/`: imágenes de las capas y vistas tridimensionales de la PCB.

## Distribución de la PCB

### Vista superior

![Vista superior de la PCB](Imagenes/Vista%20superior%20.png)

*Figura 1. Vista superior del prototipo de la PCB.*

En la vista superior se observa la distribución de los componentes que forman
el circuito ciclador.

En el centro de la placa se encuentra el ESP32, encargado de ejecutar el
algoritmo de control y generar las señales correspondientes a los procesos de
carga y descarga.

En la parte superior derecha se encuentran los transistores de potencia `Q1` y
`Q2`. Cada transistor posee un disipador de calor, debido a que estos
componentes controlan la corriente del circuito y pueden producir pérdidas de
potencia durante su funcionamiento.

Los transistores tienen las siguientes funciones:

- `Q1`: controla una de las etapas del ciclador.
- `Q2`: controla la etapa complementaria.

Debajo de los transistores se encuentran los inductores `L1` y `L2`. Estos
componentes permiten limitar las variaciones bruscas de corriente y forman
parte de las trayectorias de carga y descarga de la batería.

En la parte inferior de la PCB se encuentran los circuitos integrados `IC1` e
`IC2`, junto con las resistencias `R1`, `R2`, `R3` y `R4`. Estos componentes
forman las etapas de amplificación de las señales de control provenientes del
ESP32.

Las señales de control del ESP32 se pueden medir en los puntos:

- `S.Carga`: señal de control para la etapa de carga.
- `S.Descarga`: señal de control para la etapa de descarga.

Las señales después de las etapas de amplificación pueden medirse en:

- `Out_C.Ampli`: salida amplificada de la etapa de carga.
- `Out_D.Ampli`: salida amplificada de la etapa de descarga.

También se dispone de puntos de prueba identificados como `V+` y `GND`, los
cuales permiten comprobar las tensiones de alimentación con respecto a la
referencia común del circuito.

### Vista inferior

![Vista inferior de la PCB](Imagenes/Vista%20inferior.png)

*Figura 2. Vista inferior del prototipo de la PCB.*

La vista inferior permite observar los puntos de soldadura de los conectores,
los sensores, el ESP32, los transistores y los demás componentes instalados en
la placa.

En esta cara también se pueden verificar las conexiones entre los módulos
externos y la PCB. Esta vista resulta útil durante el ensamblaje para revisar la
orientación de los pines y comprobar que no existan uniones o cortocircuitos
entre puntos cercanos.

## Conexiones del prototipo

### Alimentación principal

La alimentación completa de la PCB se conecta mediante el conector negro
ubicado en la esquina superior izquierda.

Este conector suministra la tensión principal utilizada por las etapas de
potencia y por los demás circuitos de la placa.

Antes de conectar la alimentación se debe verificar:

- La tensión de la fuente.
- La polaridad del conector.
- La referencia común de tierra.
- Que no existan cortocircuitos entre alimentación y `GND`.

### Convertidor de 15 V a 5 V

En el lado izquierdo de la placa se encuentran dos conectores verdes pequeños
destinados a conectar un convertidor externo de tensión de `15 V` a `5 V`.

La conexión se realiza de la siguiente forma:

1. La salida de `15 V` de la PCB se conecta a la entrada del convertidor.
2. El convertidor reduce la tensión de `15 V` a `5 V`.
3. La salida de `5 V` del convertidor se conecta nuevamente a la PCB.
4. Esta tensión de `5 V` se utiliza para alimentar los sensores.

Los conectores están identificados mediante las etiquetas:

- `Out 15V`
- `IN DC/DC`

Se debe respetar la polaridad positiva y negativa indicada en la serigrafía de
la placa.

El convertidor no debe conectarse de forma inversa, ya que una tensión de
`15 V` aplicada directamente a los sensores podría dañarlos.

### Conexión de la batería

La batería se conecta en la terminal verde identificada como `J4`, ubicada en
la parte inferior derecha de la placa.

La polaridad se encuentra marcada sobre la PCB:


+ Batería -


El terminal positivo de la batería debe conectarse al punto marcado con `+` y
el terminal negativo al punto marcado con `-`.

Antes de realizar esta conexión se recomienda comprobar la polaridad con un
multímetro.

### Sensores de corriente

Los sensores de corriente se conectan en serie con la trayectoria que se desea
medir.

Para realizar esta conexión se debe abrir el conductor por el que circula la
corriente e insertar el sensor entre los dos extremos del circuito. De esta
forma, toda la corriente de carga o descarga atraviesa el sensor.

Las terminales negras ubicadas en la parte superior de cada módulo permiten
realizar esta conexión de potencia.

De manera general, la conexión es:


Salida de la etapa → Sensor de corriente → Batería o carga


Cada sensor también dispone de conexiones para:

* Alimentación de `5 V`.
* Tierra o `GND`.
* Señal analógica de salida.

La señal analógica generada por el sensor se envía al ESP32 para que el sistema
de control pueda conocer la corriente real del circuito.

Se utilizan sensores independientes para medir las corrientes asociadas con
las etapas de carga y descarga.

### Sensor de tensión

El sensor de tensión se conecta directamente en paralelo con la batería.

La conexión se realiza de la siguiente forma:


Positivo del sensor → Positivo de la batería
Negativo del sensor → Negativo de la batería


A diferencia del sensor de corriente, el sensor de tensión no requiere abrir la
trayectoria de potencia, debido a que mide la diferencia de potencial entre los
terminales de la batería.

La salida analógica del sensor debe conectarse a la entrada correspondiente del
ESP32.

La tensión entregada al ESP32 debe mantenerse dentro de su rango permitido de
entrada, por lo que el sensor debe proporcionar una señal escalada entre
`0 V` y `3.3 V`.

## Conexión general del sistema

La conexión final del prototipo se puede resumir de la siguiente manera:


Fuente principal
       |
       v
Conector negro de alimentación
       |
       +----------------------+
       |                      |
       v                      v
Etapas de potencia     Salida de 15 V
                              |
                              v
                    Convertidor de 15 V a 5 V
                              |
                              v
                    Alimentación de sensores

ESP32
  |
  +--> Señal de carga --> Amplificador --> Transistor de carga
  |
  +--> Señal de descarga --> Amplificador --> Transistor de descarga

Etapa de carga o descarga
  |
  v
Sensor de corriente
  |
  v
Batería

Sensor de tensión
  |
  +--> Conectado directamente a los terminales de la batería


El ESP32 recibe las señales de los sensores de corriente y tensión. Con estas
mediciones calcula la acción de control y genera las señales `S.Carga` y
`S.Descarga`.

Estas señales pasan por las etapas de amplificación y posteriormente controlan
los transistores de potencia.

## Capa superior de cobre

![Capa superior F.Cu](Imagenes/F.Cu.jpeg)

*Figura 3. Capa superior de cobre `F.Cu`.*

La capa `F.Cu` contiene una parte importante de las conexiones entre el ESP32,
las etapas de amplificación, los transistores, los inductores y los conectores
externos.

En esta imagen se observan pistas más anchas en las trayectorias asociadas con
la corriente de carga y descarga. Estas pistas deben soportar una corriente
mayor que las señales de control.

También se observan pistas más delgadas utilizadas para las señales del ESP32,
los sensores y los amplificadores operacionales.

## Primera capa interna

![Primera capa interna In1.Cu](Imagenes/In1.Cu.jpeg)

*Figura 4. Primera capa interna `In1.Cu`.*

La primera capa interna contiene una zona de cobre que ayuda a distribuir una
de las referencias eléctricas de la placa.

El uso de una capa interna permite reducir la longitud de las conexiones,
mejorar los caminos de retorno y disminuir el ruido producido por la etapa de
potencia.

## Segunda capa interna

![Segunda capa interna In2.Cu](Imagenes/In2.Cu.jpeg)

*Figura 5. Segunda capa interna `In2.Cu`.*

La segunda capa interna complementa la distribución de alimentación y tierra
de la PCB.

Las capas internas permiten mantener una referencia más uniforme para el ESP32,
los sensores y las etapas de acondicionamiento de señal.

## Capa inferior de cobre

![Capa inferior B.Cu](Imagenes/B.Cu.jpeg)

*Figura 6. Capa inferior de cobre `B.Cu`.*

La capa `B.Cu` completa las conexiones que no pudieron realizarse en la capa
superior.

En esta imagen se observan las conexiones inferiores entre:

* El ESP32.
* Los circuitos integrados.
* Los transistores.
* Los inductores.
* Los sensores.
* La batería.
* Los conectores de alimentación.

La combinación de las cuatro capas permite separar las señales de control de
las trayectorias de potencia y facilita la distribución de las conexiones en
toda la placa.

## Secuencia recomendada de conexión

Para conectar el prototipo se recomienda seguir este orden:

1. Mantener desconectada la batería.
2. Conectar el convertidor externo de `15 V` a `5 V`.
3. Verificar que la salida del convertidor sea realmente de `5 V`.
4. Conectar los sensores de corriente en serie con sus respectivas
   trayectorias.
5. Conectar el sensor de tensión directamente a la batería.
6. Conectar las señales de salida de los sensores al ESP32.
7. Instalar el ESP32 sobre la PCB.
8. Conectar la fuente principal al conector negro.
9. Verificar las tensiones en `V+`, `5 V` y `GND`.
10. Verificar las señales `S.Carga`, `S.Descarga`, `Out_C.Ampli` y
    `Out_D.Ampli`.
11. Conectar la batería respetando su polaridad.
12. Iniciar las pruebas con una corriente limitada.

## Uso de los archivos

Para abrir o modificar el proyecto se debe utilizar el archivo:


Proyecto.kicad_pro


Este archivo abre el proyecto completo dentro de KiCad.

El esquemático puede modificarse desde:


Proyecto.kicad_sch


La distribución de componentes y pistas puede modificarse desde:


Proyecto.kicad_pcb


También deben mantenerse las carpetas `symbols`, `footprints` y `3dmodels`,
debido a que contienen elementos utilizados por el proyecto.




