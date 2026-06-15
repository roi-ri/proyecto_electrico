Diseño del circuito
Se implementa la topología mostrada en la Figura 1. En el lado izquierdo se encuentra el circuito encargado de la carga de la batería, mientras que en el lado derecho se ubica el circuito de descarga. Ambas etapas presentan una estructura similar, debido a que en los dos casos se requiere controlar la corriente que circula hacia o desde la batería.
Al analizar la trayectoria de corriente del circuito de carga en sentido horario, primero se encuentra la fuente de tensión que proporciona la energía necesaria para cargar la batería. A continuación, se ubica el transistor de potencia, el cual funciona como un elemento de conmutación que regula el paso de corriente. Desde el punto de vista del sistema de control, este transistor corresponde al actuador, ya que modifica la corriente del circuito de acuerdo con la señal de control aplicada.
Posteriormente, se encuentra el inductor, cuya función es limitar las variaciones bruscas de corriente y almacenar energía durante la operación del convertidor. Debido a la relación existente entre la tensión aplicada al inductor y la variación de su corriente, es posible controlar la corriente de carga o descarga mediante la conmutación del transistor.
Adicionalmente, el circuito cuenta con una etapa de acondicionamiento de la señal de control proveniente del ESP32. Esta señal posee un rango de tensión entre 0 V y 3.3 V, por lo que se introduce en un amplificador operacional configurado como amplificador no inversor con una ganancia de 4. De esta manera, la señal de salida puede alcanzar un valor máximo aproximado de 13.2 V.
Esta amplificación es necesaria para proporcionar un nivel de tensión adecuado para el accionamiento del transistor. Durante los procesos de carga y descarga, la tensión de control debe ser suficientemente alta con respecto al nodo ubicado entre el transistor y el inductor, con el fin de garantizar la correcta activación del dispositivo de conmutación y permitir el control de la corriente que circula por la batería.


## Diagrama del circuito

![Circuito de carga y descarga de la batería](Imagenes/circuito.PNG)

*Figura 1. Circuito de carga y descarga de la batería.*

