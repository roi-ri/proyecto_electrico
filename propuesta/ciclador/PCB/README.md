````markdown
# PCB del ciclador de baterías

## Descripción general

Este directorio contiene el diseño de la placa de circuito impreso del
ciclador de baterías. La PCB integra la etapa de control, las etapas de carga
y descarga, el acondicionamiento de señales y las conexiones necesarias para
interactuar con la batería y con los dispositivos externos del sistema.

El diseño fue realizado en KiCad y está planteado como una PCB de cuatro capas.
Las capas externas contienen gran parte del enrutamiento de las señales y de
las trayectorias de potencia, mientras que las capas internas pueden utilizarse
como planos de referencia y alimentación, de acuerdo con la asignación de redes
definida en el proyecto.

El sistema permite conectar los siguientes elementos:

- Una fuente para la etapa de carga.
- Una carga o fuente externa para la etapa de descarga.
- La batería que será sometida al proceso de ciclado.
- El ESP32 encargado del sistema de control.
- Las etapas de amplificación de las señales de control.
- Los sensores utilizados para medir corriente y tensión.

## Contenido del directorio

La estructura principal del proyecto es la siguiente:

```text
PCB/
├── Imagenes/
│   ├── vista_superior.png
│   ├── vista_inferior.png
│   ├── f_cu.jpeg
│   ├── b_cu.jpeg
│   ├── in1_cu.jpeg
│   └── in2_cu.jpeg
├── 3dmodels/
├── footprints/
├── outputs/
├── symbols/
├── Proyecto.kicad_pro
├── Proyecto.kicad_sch
├── Proyecto.kicad_pcb
└── README.md
```

### Archivos principales

- `Proyecto.kicad_pro`: archivo principal del proyecto de KiCad.
- `Proyecto.kicad_sch`: contiene el esquemático eléctrico del circuito.
- `Proyecto.kicad_pcb`: contiene el diseño físico de la PCB.
- `symbols/`: contiene los símbolos personalizados utilizados en el
  esquemático.
- `footprints/`: contiene las huellas personalizadas utilizadas en la PCB.
- `3dmodels/`: contiene los modelos tridimensionales de los componentes.
- `outputs/`: puede utilizarse para almacenar los archivos Gerber, archivos de
  perforación, listas de materiales y demás archivos de fabricación.
- `Imagenes/`: contiene las vistas de la PCB y de sus diferentes capas.

## Distribución general de la PCB

La PCB se divide en varias secciones funcionales. En el lado izquierdo se
encuentran las conexiones de alimentación, las señales de control y el módulo
ESP32. En la parte inferior se ubican las etapas de acondicionamiento de las
señales. En el lado derecho se encuentran los transistores de potencia, los
inductores y las terminales asociadas con la carga, la descarga y la batería.

### Vista superior

La siguiente imagen muestra la distribución de los componentes sobre la cara
superior de la PCB.

![Vista superior de la PCB](Imagenes/vista_superior.png)

*Figura 1. Vista superior tridimensional de la PCB.*

En esta vista se pueden identificar los siguientes elementos:

- El módulo ESP32, encargado de ejecutar el sistema de control.
- Los transistores de potencia `Q1` y `Q2`.
- Los disipadores de calor de los transistores.
- Los inductores `L1` y `L2`.
- Los circuitos integrados `IC1` e `IC2`.
- Las resistencias asociadas con las etapas de amplificación.
- Las terminales de entrada y salida.
- La terminal de conexión de la batería.
- Los puntos de prueba para las señales de carga y descarga.
- Los agujeros de montaje ubicados en las esquinas de la PCB.

Los transistores se colocaron cerca del borde superior para facilitar la
instalación de los disipadores y mejorar la evacuación del calor. Los
inductores y las terminales de potencia se encuentran próximos a los
transistores para reducir la longitud de las trayectorias por las que circulan
corrientes elevadas.

El ESP32 se ubica separado de la etapa de potencia para disminuir la
interferencia eléctrica sobre las señales de control y medición.

### Vista inferior

La vista inferior permite observar las conexiones, terminales y puntos de
soldadura accesibles desde la parte posterior de la placa.

![Vista inferior de la PCB](Imagenes/vista_inferior.png)

*Figura 2. Vista inferior tridimensional de la PCB.*

En esta cara se observan principalmente:

- Los pines de conexión del ESP32.
- Las terminales de los transistores.
- Las conexiones de los inductores.
- Los puntos de soldadura de las borneras.
- Las conexiones de las entradas y salidas.
- Los agujeros de montaje de la placa.

Esta vista debe revisarse antes del ensamblaje para confirmar la orientación de
los conectores y evitar inversiones de polaridad durante la soldadura.

## Secciones principales del circuito

### ESP32 y sistema de control

El ESP32 se conecta a la PCB mediante dos hileras de pines. Este
microcontrolador recibe las señales simuladas por los sensores, ejecuta el
algoritmo de control y genera las señales necesarias para regular las etapas de
carga y descarga.

Las señales identificadas como `S.Carga` y `S.Descarga` corresponden a las
señales de control utilizadas para accionar cada etapa del ciclador.

Antes de conectar el ESP32 se debe verificar:

- Que los pines coincidan con la distribución definida en el esquemático.
- Que las señales no superen el rango permitido por sus entradas analógicas.
- Que exista una referencia de tierra común.
- Que las salidas de control tengan el escalamiento correcto.
- Que la alimentación del módulo tenga la polaridad adecuada.

### Etapas de amplificación

Los circuitos integrados `IC1` e `IC2`, junto con las resistencias `R1`, `R2`,
`R3` y `R4`, forman las etapas de acondicionamiento o amplificación de las
señales provenientes del ESP32.

Estas etapas adaptan las señales de control de bajo nivel a los valores
requeridos por los transistores de potencia.

Los puntos identificados como `Out_C.Ampli` y `Out_D.Ampli` permiten medir las
salidas de las etapas de amplificación de carga y descarga, respectivamente.

Estos puntos pueden utilizarse durante las pruebas para comprobar:

- El valor máximo de la señal amplificada.
- La linealidad de la etapa.
- La ausencia de saturación del amplificador.
- La respuesta ante cambios en la señal de control.
- La correcta referencia respecto a tierra.

### Transistores de potencia

Los componentes `Q1` y `Q2` controlan el paso de corriente en las etapas de
carga y descarga.

Cada transistor dispone de un disipador de calor debido a las pérdidas de
potencia que pueden presentarse durante la operación. Antes de utilizar el
circuito se debe verificar que:

- El encapsulado coincida con la huella de la PCB.
- La distribución de terminales sea correcta.
- El disipador no produzca cortocircuitos.
- La corriente máxima permanezca dentro de los límites del transistor.
- La temperatura de operación sea segura.
- El transistor cuente con el aislamiento requerido respecto al disipador.

### Inductores

Los inductores `L1` y `L2` forman parte de las etapas de potencia y limitan las
variaciones rápidas de corriente.

Uno de los inductores corresponde a la trayectoria de carga y el otro a la
trayectoria de descarga. Su ubicación cercana a los transistores y conectores
de potencia reduce la longitud de los lazos de corriente.

Los inductores seleccionados deben soportar:

- La corriente máxima del sistema.
- La corriente de saturación.
- Las pérdidas en el núcleo.
- El calentamiento producido durante los ciclos.
- La frecuencia de operación del sistema de control.

### Conexión de la batería

La batería se conecta mediante la terminal identificada como `J4`.

La serigrafía indica la polaridad mediante las marcas:

```text
+ Batería -
```

Antes de conectar una batería física se debe confirmar la polaridad con un
multímetro. Una inversión de polaridad puede dañar los transistores, los
sensores, el ESP32 o las pistas de la PCB.

### Entradas y salidas de potencia

La placa incluye terminales para conectar las fuentes y los elementos externos
del circuito. Entre las etiquetas visibles se encuentran:

- `IN DC/DC`
- `Out 15V`
- `Batería`
- `V+`
- `GND`

La función exacta de cada terminal debe verificarse con el esquemático antes de
energizar la PCB.

No se debe asumir la polaridad únicamente por la posición física del conector.
Siempre debe revisarse la serigrafía y realizarse una prueba de continuidad.

## Capas de la PCB

El diseño utiliza cuatro capas de cobre:

1. `F.Cu`: capa de cobre superior.
2. `In1.Cu`: primera capa interna.
3. `In2.Cu`: segunda capa interna.
4. `B.Cu`: capa de cobre inferior.

### Capa superior — F.Cu

![Capa superior de cobre](Imagenes/f_cu.jpeg)

*Figura 3. Enrutamiento de la capa superior de cobre.*

La capa `F.Cu` contiene conexiones entre los componentes ubicados en la cara
superior de la placa.

En esta capa se pueden observar:

- Las conexiones de los transistores.
- Las trayectorias hacia los inductores.
- Las conexiones del ESP32.
- Las pistas de las etapas de amplificación.
- Las conexiones hacia las borneras.
- Las trayectorias de corriente de carga y descarga.

Las pistas asociadas con la etapa de potencia deben ser suficientemente anchas
para soportar la corriente máxima prevista. Las pistas de medición y control
pueden utilizar anchos menores, ya que transportan corrientes reducidas.

### Primera capa interna — In1.Cu

![Primera capa interna](Imagenes/in1_cu.jpeg)

*Figura 4. Primera capa interna de la PCB.*

La capa `In1.Cu` puede utilizarse como plano de referencia o como plano de
alimentación, dependiendo de la asignación de redes realizada en KiCad.

El uso de una capa interna continua permite:

- Reducir la impedancia de retorno.
- Mejorar la distribución de corriente.
- Disminuir el ruido eléctrico.
- Facilitar la conexión a tierra de diferentes secciones.
- Reducir el área de los lazos de corriente.

La red asignada a esta capa debe comprobarse directamente en el archivo
`Proyecto.kicad_pcb`.

### Segunda capa interna — In2.Cu

![Segunda capa interna](Imagenes/in2_cu.jpeg)

*Figura 5. Segunda capa interna de la PCB.*

La segunda capa interna complementa la distribución de alimentación y retorno
del circuito.

Antes de fabricar la placa se debe verificar que las zonas de cobre estén
actualizadas y que no existan islas de cobre sin conexión.

En KiCad, las zonas pueden actualizarse presionando la tecla:

```text
B
```

### Capa inferior — B.Cu

![Capa inferior de cobre](Imagenes/b_cu.jpeg)

*Figura 6. Enrutamiento de la capa inferior de cobre.*

La capa `B.Cu` completa las conexiones que no pueden realizarse en la capa
superior y permite distribuir las señales entre las diferentes secciones de la
PCB.

En esta capa se observan pistas que conectan:

- El ESP32 con las etapas de control.
- Las etapas de amplificación con los transistores.
- Los inductores con las terminales externas.
- La conexión de la batería.
- Las referencias de alimentación y tierra.

## Descarga del proyecto

### Opción 1: descargar desde GitHub

Para descargar el proyecto completo desde la página de GitHub:

1. Ingresar al repositorio.
2. Presionar el botón `Code`.
3. Seleccionar `Download ZIP`.
4. Descomprimir el archivo descargado.
5. Ingresar al directorio:

```text
propuesta/ciclador/PCB
```

Es importante descargar el repositorio completo y no únicamente los archivos
`.kicad_pcb` o `.kicad_sch`, debido a que el proyecto utiliza carpetas
adicionales para símbolos, huellas y modelos 3D.

### Opción 2: clonar el repositorio

Desde una terminal se puede ejecutar:

```bash
git clone <URL_DEL_REPOSITORIO>
cd proyecto_electrico/propuesta/ciclador/PCB
```

Para descargar cambios posteriores:

```bash
git pull
```

## Edición del proyecto

### Requisitos

Para abrir y editar el diseño se necesita KiCad. Se recomienda utilizar una
versión compatible con la empleada para crear el proyecto.

### Abrir el proyecto en macOS

Desde la carpeta `PCB`:

```bash
open Proyecto.kicad_pro
```

También se puede abrir KiCad y seleccionar manualmente:

```text
Archivo → Abrir proyecto existente
```

Luego se debe elegir:

```text
Proyecto.kicad_pro
```

### Archivos que deben editarse

Para modificar el esquemático:

```text
Proyecto.kicad_sch
```

Para modificar la distribución y el enrutamiento de la PCB:

```text
Proyecto.kicad_pcb
```

Se recomienda abrir siempre `Proyecto.kicad_pro`, ya que este archivo carga la
configuración completa del proyecto.

### Bibliotecas personalizadas

Si KiCad indica que no encuentra símbolos o huellas, se deben configurar las
bibliotecas incluidas en el repositorio.

Para los símbolos:

```text
Preferencias → Administrar bibliotecas de símbolos
```

Se debe agregar la biblioteca correspondiente ubicada dentro de:

```text
symbols/
```

Para las huellas:

```text
Preferencias → Administrar bibliotecas de huellas
```

Se debe agregar la carpeta correspondiente ubicada dentro de:

```text
footprints/
```

Se recomienda utilizar rutas relativas al proyecto mediante:

```text
${KIPRJMOD}
```

Por ejemplo:

```text
${KIPRJMOD}/footprints
${KIPRJMOD}/symbols
${KIPRJMOD}/3dmodels
```

Esto permite que el proyecto pueda abrirse en otras computadoras sin tener que
modificar rutas absolutas.

## Verificaciones antes de fabricar

Antes de generar los archivos de fabricación se deben realizar las siguientes
comprobaciones:

### Revisión del esquemático

En el editor de esquemáticos se debe ejecutar la comprobación de reglas
eléctricas:

```text
Inspeccionar → Comprobador de reglas eléctricas
```

Se deben revisar especialmente:

- Pines sin conexión.
- Salidas conectadas entre sí.
- Entradas flotantes.
- Pines de alimentación no reconocidos.
- Conexiones sin referencia de tierra.
- Numeración y valores de los componentes.

### Revisión de la PCB

En el editor de PCB se debe ejecutar:

```text
Inspeccionar → Comprobador de reglas de diseño
```

Se deben corregir errores relacionados con:

- Pistas sin conectar.
- Separaciones insuficientes.
- Vías demasiado cercanas.
- Pistas fuera del borde de la placa.
- Taladros superpuestos.
- Componentes fuera de la PCB.
- Errores entre zonas de cobre.
- Incompatibilidad entre huellas y encapsulados.

También se debe comprobar:

- El ancho de las pistas de potencia.
- La capacidad de corriente de las vías.
- La separación entre potencia y control.
- La orientación de los conectores.
- La polaridad de la batería.
- La orientación de los circuitos integrados.
- La distribución de terminales de los transistores.
- La posición de los disipadores.
- La distancia entre componentes y agujeros de montaje.

## Generación de archivos de fabricación

Para fabricar la PCB se deben generar los archivos Gerber y los archivos de
perforación.

### Archivos Gerber

Desde el editor de PCB:

```text
Archivo → Trazar
```

Para una PCB de cuatro capas se deben seleccionar, como mínimo:

```text
F.Cu
In1.Cu
In2.Cu
B.Cu
F.Mask
B.Mask
F.SilkS
B.SilkS
Edge.Cuts
```

Las capas de pasta:

```text
F.Paste
B.Paste
```

solamente son necesarias cuando se solicitará una plantilla para aplicar pasta
de soldadura.

Los archivos pueden guardarse en:

```text
outputs/gerbers/
```

### Archivos de perforación

Dentro de la misma ventana se debe seleccionar:

```text
Generar archivos de taladrado
```

Se recomienda utilizar el formato Excellon, que es aceptado por la mayoría de
los fabricantes.

Los archivos de perforación también deben colocarse en:

```text
outputs/gerbers/
```

### Verificación de los Gerber

Antes de enviar los archivos al fabricante se deben abrir en el visor Gerber de
KiCad:

```text
KiCad → Visor Gerber
```

Se debe verificar que:

- El borde de la placa esté cerrado.
- Las cuatro capas de cobre estén presentes.
- Los agujeros estén correctamente ubicados.
- La máscara de soldadura no cubra los pads.
- La serigrafía no se encuentre sobre pads expuestos.
- Los textos tengan una orientación correcta.
- Las zonas de cobre estén rellenas.
- No existan pistas cortadas.
- Las dimensiones de la placa sean las esperadas.

## Preparación para fabricación

Una vez verificados los archivos, el contenido de la carpeta de Gerbers debe
comprimirse:

```bash
cd outputs
zip -r pcb_ciclador_gerbers.zip gerbers
```

El archivo generado puede cargarse en la página del fabricante seleccionado.

Al configurar el pedido se debe indicar que la PCB utiliza cuatro capas. Los
demás parámetros deben definirse de acuerdo con los requerimientos eléctricos
y mecánicos del proyecto.

Entre los parámetros que deben revisarse se encuentran:

- Número de capas.
- Espesor de la PCB.
- Espesor de cobre.
- Material de la placa.
- Acabado superficial.
- Color de la máscara de soldadura.
- Color de la serigrafía.
- Diámetro mínimo de perforación.
- Ancho y separación mínima de pistas.
- Dimensiones de la PCB.
- Cantidad de unidades.
- Necesidad de plantilla de soldadura.

No se deben seleccionar automáticamente los valores más económicos sin
comprobar que soporten las corrientes del circuito.

## Ensamblaje inicial

Antes de soldar todos los componentes se recomienda seguir este orden:

1. Revisar visualmente la PCB recibida.
2. Realizar pruebas de continuidad.
3. Verificar que no existan cortocircuitos entre alimentación y tierra.
4. Soldar primero los componentes pequeños.
5. Soldar los circuitos integrados.
6. Soldar las resistencias y conectores.
7. Soldar los inductores.
8. Instalar los transistores.
9. Instalar los disipadores.
10. Conectar el ESP32 únicamente después de verificar las tensiones.
11. Probar las fuentes sin conectar una batería.
12. Realizar la primera prueba con una fuente limitada en corriente.

## Pruebas recomendadas

Antes de conectar una batería real se recomienda verificar:

- Continuidad de todas las conexiones de potencia.
- Ausencia de cortocircuitos.
- Tensión de alimentación del ESP32.
- Tensión de salida de las etapas de amplificación.
- Señales de carga y descarga.
- Funcionamiento de los sensores.
- Polaridad de los conectores.
- Respuesta de los transistores.
- Temperatura de los componentes de potencia.
- Corriente máxima del circuito.
- Comunicación entre el ESP32 y los demás dispositivos.

Las primeras pruebas deben realizarse con una fuente de laboratorio limitada en
corriente. La batería solamente debe conectarse después de comprobar que las
etapas de control, medición y potencia funcionan correctamente.

## Advertencia

Este diseño corresponde a un prototipo académico. Antes de utilizarlo con una
batería real se deben verificar todas las conexiones, límites de tensión,
límites de corriente, temperaturas y mecanismos de protección.

Una conexión incorrecta o un error en el sistema de control puede provocar
daños en la PCB, en el ESP32, en los componentes de potencia o en la batería.
````

