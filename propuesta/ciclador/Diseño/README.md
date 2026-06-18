## Diseño del circuito

La topología implementada se muestra en la **Figura 1**. En el lado izquierdo se encuentra el circuito encargado de la carga de la batería, mientras que en el lado derecho se ubica el circuito de descarga.

Ambas etapas presentan una estructura similar, debido a que en los dos casos se requiere controlar la corriente que circula hacia o desde la batería.

### 1. Etapa de carga

Al analizar la trayectoria de corriente del circuito de carga en sentido horario, se encuentran los siguientes elementos:

1. **Fuente de tensión:** proporciona la energía necesaria para cargar la batería.
2. **Transistor de potencia:** regula el paso de corriente mediante su conmutación.
3. **Inductor:** limita las variaciones bruscas de corriente y almacena energía.
4. **Batería:** recibe la corriente suministrada por el circuito de carga.

Desde el punto de vista del sistema de control, el transistor corresponde al **actuador**, ya que modifica la corriente del circuito según la señal de control aplicada.

### 2. Etapa de descarga

La etapa de descarga presenta una estructura semejante a la etapa de carga. En este caso, el transistor regula la corriente que sale de la batería, mientras que el inductor limita sus variaciones bruscas.

Debido a la relación entre la tensión aplicada al inductor y la variación de su corriente, es posible controlar la corriente de carga o descarga mediante la conmutación del transistor.

### 3. Acondicionamiento de la señal de control

La señal de control proveniente del ESP32 posee un rango de tensión entre **0 V y 3.3 V**.

Esta señal se introduce en un amplificador operacional configurado como **amplificador no inversor**, formado por resistencias de **1 kΩ** y **3.3 kΩ**. La ganancia del amplificador es:

$$
A_v = 1 + \frac{3\ \text{k}\Omega}{1\ \text{k}\Omega} = 4.3
$$

Por lo tanto, la tensión máxima de salida es aproximadamente:

$$
V_{\text{salida}} = 4.3(3.3\ \text{V}) = 14.2\ \text{V}
$$

### 4. Accionamiento del transistor

La amplificación es necesaria para proporcionar un nivel de tensión adecuado para el accionamiento del transistor.

Durante los procesos de carga y descarga, la tensión de control debe ser suficientemente alta con respecto al nodo ubicado entre el transistor y el inductor. Esto permite:

* Garantizar la correcta activación del transistor.
* Regular la corriente del circuito.
* Controlar la corriente que circula hacia o desde la batería.

## Diagrama del circuito

![Circuito de carga y descarga de la batería](Imagenes/circuito.png)

*Figura 1. Topología del circuito de carga y descarga de la batería.*



## Función de transferencia

### 1. Circuito de carga

Aplicando la **Ley de Voltajes de Kirchhoff** en sentido horario sobre el
circuito de carga, se obtiene:

$$
-v_C(t)+v_T(t)+v_L(t)+v_R(t)+v_B(t)=0
$$

La tensión en el inductor se expresa como:

$$
v_L(t)=L\frac{di_L(t)}{dt}
$$

Por otra parte, la caída de tensión en la resistencia parásita del circuito
está dada por:

$$
v_R(t)=R\cdot i_L(t)
$$

Sustituyendo ambas expresiones en la ecuación de tensiones:

$$
-v_C(t)+v_T(t)+L\frac{di_L(t)}{dt}
+R\cdot i_L(t)+v_B(t)=0
$$

Al reorganizar los términos:

$$
L\frac{di_L(t)}{dt}+R\cdot i_L(t) = v_C(t)-v_T(t)-v_B(t)
$$

Aplicando la transformada de Laplace con condiciones iniciales iguales a
cero:

$$
LsI_L(s)+RI_L(s) = V_C(s)-V_T(s)-V_B(s)
$$

Factorizando la corriente del inductor:

$$
I_L(s)(Ls+R) = V_C(s)-V_T(s)-V_B(s)
$$

Se define la entrada efectiva del circuito de carga como:

$$
U_C(s)=V_C(s)-V_T(s)-V_B(s)
$$

Por lo tanto, la corriente del inductor se expresa como:

$$
I_L(s)=\frac{U_C(s)}{Ls+R}
$$

Finalmente, la función de transferencia del circuito de carga es:

$$
H_C(s) = \frac{I_L(s)}{U_C(s)} = \frac{1}{Ls+R}
$$

### 2. Circuito de descarga

El análisis del circuito de descarga se realiza de manera semejante al del
circuito de carga. Sin embargo, durante la descarga la batería funciona como
fuente de energía y la corriente circula en sentido contrario.

Para comparar ambas etapas se mantiene como referencia positiva el sentido de
la corriente utilizado en el circuito de carga. Por esta razón, la corriente
de descarga aparece con signo negativo.

En este caso, la ecuación diferencial puede escribirse como:

$$
L\frac{di_L(t)}{dt}+R\cdot i_L(t) = v_T(t)+v_D(t)-v_B(t)
$$

donde $v_D(t)$ representa la tensión presente en la salida del circuito de
descarga.

Aplicando la transformada de Laplace con condiciones iniciales iguales a cero:

$$
LsI_L(s)+RI_L(s) = V_T(s)+V_D(s)-V_B(s)
$$

Factorizando la corriente del inductor:

$$
I_L(s)(Ls+R) = V_T(s)+V_D(s)-V_B(s)
$$

Se define la entrada efectiva del circuito de descarga como:

$$
U_D(s)=V_B(s)-V_T(s)-V_D(s)
$$

Debido a que:

$$
V_T(s)+V_D(s)-V_B(s)=-U_D(s)
$$

la corriente del inductor queda expresada como:

$$
I_L(s)=-\frac{U_D(s)}{Ls+R}
$$

Por lo tanto, la función de transferencia del circuito de descarga es:

$$
H_D(s) = \frac{I_L(s)}{U_D(s)} = -\frac{1}{Ls+R}
$$

El signo negativo indica que la corriente de descarga circula en sentido
contrario a la referencia de corriente definida para el proceso de carga.











