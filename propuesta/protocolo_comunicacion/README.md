# Propuesta para protocolo de comunicación 

## Objetivo del sistema

Desarrollar un sistema de control basado en un microcontrolador que permita la interacción con una computadora mediante un protocolo de comunicación definido, facilitando el intercambio de información y el control de dispositivos del sistema, de manera que se establezca una comunicación eficiente entre el usuario y el controlador, que permita:

- Recibir y decodificar comandos enviados desde una interfaz de usuario en la computadora mediante el protocolo de comunicación establecido.  
- Procesar las solicitudes recibidas para ejecutar acciones sobre sensores o dispositivos conectados al sistema.  
- Adquirir datos provenientes de sensores o del estado interno del sistema para su transmisión hacia la computadora.  
- Enviar información de retorno hacia la computadora, incluyendo estados del sistema, confirmación de comandos ejecutados y datos adquiridos.  
- Organizar el intercambio de información mediante una estructura de mensajes o tramas que garantice la correcta interpretación de los datos transmitidos.

## Alcances

El sistema contempla el desarrollo de un controlador basado en un microcontrolador y un protocolo de comunicación definido para la interacción con una computadora, que pueda realizar lo siguiente:
- Comunicación bidireccional entre la computadora y el controlador mediante un protocolo de comunicación previamente definido.
- Recepción y procesamiento de comandos provenientes de una interfaz de usuario en la computadora.
- Interpretación de los datos recibidos para ejecutar acciones específicas dentro del sistema de control.
- Envío de información desde el controlador hacia la computadora, incluyendo estados del sistema, confirmación de comandos y datos adquiridos de sensores.
- Integración de sensores o dispositivos externos que permitan la adquisición de información relevante para el funcionamiento del sistema.
- Implementación de un esquema de control que permita ejecutar las acciones solicitadas por el usuario de manera estructurada.
- Organización del intercambio de datos mediante una estructura de mensajes que facilite la interpretación y manejo de la información transmitida.
- Visualización en la computadora de los datos enviados por el controlador para el monitoreo del sistema.


## Limitaciones

En esta etapa no se contempla:
- Desarrollo o modificación del hardware asociado a los sensores o dispositivos que interactúan con el controlador.
- Implementación de múltiples protocolos de comunicación distintos al protocolo definido para el intercambio de datos entre la computadora y el microcontrolador.
- Ontegración con plataformas industriales de automatización, sistemas SCADA o plataformas industriales para control en tiempo real.
- Implementación de mecanismos avanzados de seguridad, cifrado o autentificación en la comunicación de datos.
- Manejo de grandes volúmenes de datos o transmisión de información de alta velocidad más allá de las capacidades del microcontrolador y del medio de comunicación utilizado,
- Escalabilidad hacia sistemas distribuidos con múltiples controladores o redes complejas de dispositivos.
- Certificación bajo normativas industriales o de seguridad específicas para aplicaciones críticas.


## Posibles protocolos de comunicación

### Protocolo UART
UART es un protocolo de comunicación seria asíncrono, lo que significa que no utiliza una señal de reloj compartida entre los dispositivos que se comunican. En su lugar, ambos dispositivos deben estar configurados con la misma velocidad de transmisión (baud rate) para interpretar correctamente los datos. La comunicación con UART se realiza generalmente utilizando dos líneas principales:

- **TX (Transmit)**: Línea de transmisión de datos.
- **RX (Receive)**: Línea de recepción de datos.

Cada dato transmitido se organiza en una trama que incluye:
- Bit de inicio (Start bit)
- Bits de datos (generalmente 8 bits)
- Bit opcional de paridad
- Bits de parada (Stop bits)


#### Ventajas
- Es full-duplex (puede transmitir y recibir datos al mismo tiempo).
- Implementación simple y ampliamente soportada por microcontroladores.
- Bajo número de pines requeridos.
- Fácil integración con interfaces de computadora.


#### Desventajas
- Velocidades menores comparadas con otros protocolos.
- No permite múltiples dispositivos en el mismo bus de forma directa.
- No posee sincronización por reloj, por lo que puede ser más susceptible a errores si las velocidades no coinciden.


Las velocidades típicas del protocolo UART van desde 9600 bps hasta varios Mbps, pero esto depende del hardware. Además UART no incluye mecanismos avanzados de verificación de errores, aunque puede emplear bits de paridad o implementaciones de software para verificar la integridad de los datos. UART necesita un convertidor para poder enviar los datos por USB.


### Protocolo SPI
SPI es un protocolo de comnicación serial sincrónico, desarrollado para permitir una comunicación rápida entre un dispositivo maestro (generalmente un microcontrolador) y uno o varios dispositivos esclavos. SPI utiliza cuatro líneas principales:
- **MOSI (Master Out Slave In)**: Datos enviados del maestro al esclavo.
- **MISO (Master In Slave Out)**: Datos enviados del esclavo al maestro.
- **SCLK (Serial Clock)**: Señal de reloj generada por el maestro,
- **CS/SS (Chip Select)**: Selección del dispositivo esclavo.

#### Ventajas
- Alta velocidad de transmisión.
- Es full-duplex (puede transmitir y recibir datos al mismo tiempo).
- Baja latencia en la transmisión de datos.

#### Desventajas
- Requiere más líneas físicas que otros protocolos.
- Cada dispositivo esclavo requiere una línea Chip Select adicional.
- No existe un estándar universal para la estructura de datos, por lo que depende de cada dispositivo.

El SPI puede alcanzar velocidades de decenas de Mbps, dependiendo del microcontrolador y del hardware utilizado. El protocolo no incluye verificación de errores integrada, por lo que se suele implementar CRC o verificación por software cuando la aplicación lo requiere. Este tampoco es compatible con USB, por lo que se necesita un convertidor SPI-USB.

### Protocolo I2C
I2C es un protocolo serial sincrónico diseñado para permitir la comunicación entre múltiples dispositivos utilizando únicamente dos líneas de comunicación:
- **SDA (Serial Data)**: Línea de datos.
- **SCL (Serial Clock)**: Línea de reloj.
A diferencia de SPI, I2C permite conectar múltiples dispositivos en el mismo bus, identificándolos mediante direcciones únicas.
#### Ventajas
- Solo requiere dos líneas para múltiples dispositivos.
- Permite comunicación multimaestro y multiesclavo.
- Amplio soporte en sensores, memorias y circuitos integrados.

#### Desventajas
- Velocidad mejor comparada con SPI.
- Mayor complejidad en la gestión del bus cuando hay muchos dispositivos.
- Distancias de comunicación relativamente cortas.

Las velocidades del I2C son variadas, ya que hay de 100kbps, 400 kbps, 1 Mbps hasta 3,4 Mbps y además este protocolo incluye mecanismos como ACK/NACK para confirmar recepción de datos y control de colisiones en sistemas multimaestro. El protocolo I2C no es compatible con usb, por lo que se necesita un adaptador I2C-USB.

## Propuesta inicial
Como propuesta inicial para el sistema de comunicación entre la computadora y el controlador se plantea la utilización del protocolo UART, debido a su simplicidad de implementación, amplia compatibilidad con microcontroladores y facilidad de integración con computadoras mediante interfaces USB.
La selección de este protocolo se fundamenta en que el sistema contempla la comunicación con un único dispositivo controlador, por lo que no es necesario implementar un bus que permita múltiples dispositivos conectados simultáneamente, como ocurre con protocolos como I2C o SPI. En este contexto, UART resulta adecuado al permitir una comunicación serial directa entre el microcontrolador y la computadora utilizando únicamente líneas de transmisión y recepción de datos, además de que UART es full-duplex, por lo que no habrá problemas de interrupción en el recibimiento o envío de datos si se desea hacer un cambio mientras la computadora recibe datos.
Adicionalmente, el uso de UART facilita el desarrollo y depuración del sistema, ya que permite visualizar y enviar datos de forma directa mediante herramientas de monitoreo serial disponibles en diferentes entornos de desarrollo o aplicaciones de comunicación serial en la computadora.
En cuanto a la interfaz física, el sistema utilizará la conexión USB disponible en la placa de desarrollo del ESP32, la cual permite establecer la comunicación con la computadora. Ya que en la placa que se utiliza el conector USB incorpora un convertidor USB-UART integrado, que actúa como intermediario entre el protocolo USB utilizado por la computadora y la comunicación serial UART del microcontrolador. Gracias a esto es posible generar simplificaciones y ahorro de costos ya que no es necesario implementar adaptadores externos adicionales, ya que la placa permite que la computadora reconozca el dispositivo como un puerto serial virtual.
De esta manera, el protocolo UART permitirá que el sistema:

- Reciba comandos enviados desde la computadora a través de la interfaz de usuario.
- Procese dichos comandos en el microcontrolador para ejecutar las acciones correspondientes en el sistema de control.
- Envíe información de retorno hacia la computadora, como estados del sistema, confirmación de comandos o datos adquiridos por los sensores.




