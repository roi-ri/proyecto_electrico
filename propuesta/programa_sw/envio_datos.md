# Estipulación de los datos a enviar desde el ESP32


Luego del Start, selección de tipo de batería y función: 

Se envía por medio de comunicación serial: 


```cpp
datos = [batería seleccionada, tipo de batería, función]
```

Donde: 

- Batería seleccionada es un 1 o 0. 
- Tipo de batería, se va a establecer un código para cada tipo batería que soporta el programa.
- función: 
    - 1: Ciclado
    - 2: Carga
    - 3: Descarga
    - 4: Leer datos .csv (aquí el ESP32 no va a hacer nada)

---

Selección de función de Ciclado: 

```cpp
datos = [Tipo de ciclado, cantidad de ciclados]
```
Donde: 
- ``datos[0]``: 
    - 0: Ciclado indefinido, ignora a ``datos[1]`` y espera una nueva señal de stop. 
    - 1: Ciclado definido, toma en cuenta la cantidad de ciclados que se indica en ``datos[1]``.
- ``datos[1]``: Indica la cantidad de ciclados que se quieren llevar a cabo. 


---

Sección de carga: 
Envía: 
```cpp
datos = [% de carga]
```
El porcentaje de carga luego se convierte en el ESP32 y se establece hasta que nivel debe llegar la batería


--- 
Sección de descarga: 
Envía: 
```cpp
datos = [% de descarga]
```
El porcentaje de carga luego se convierte en el ESP32 y se establece hasta que nivel debe llegar la batería


--- 
Leer datos .csv no envía nada. 
