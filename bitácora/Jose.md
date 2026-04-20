# Bitácora de trabajo 

Jose Manuel Solis
C27645 


| Fecha       | Horas   | Objetivos | Descripción | Resultados | Comentario |
|:-----------:|:-------:|-----------|-------------|------------|------------|
| - | - | Exponer las ideas del grupo.<br>Dibujar un modelo de lo que se desea. | Se diseña un borrador del prototipo final que se desea implementar. | Se dibuja un esquemático con las partes del prototipo. | Queda pendiente definir si utilizar un ESP32 o un C2000. |
| - | - | Diseñar un circuito y un lazo de control para ciclar la batería.<br>Simular el circuito y verificar su funcionamiento. | Se diseña el circuito del ciclador y se simula en PSpice. | La simulación cumple con cargar y descargar la batería cuando se aplica la señal al amplificador operacional mediante el sistema de control. | Falta realizar los cálculos de la señal de control para verificar que el circuito cumple con las ecuaciones de carga y descarga. |
| - | - | Calcular la acción de control de la malla de carga y descarga. | Se realizan los cálculos correspondientes para identificar la acción de control de cada malla. | La acción de control resulta como se esperaba: el circuito de carga es positivo y el de descarga negativo. | Se confirma el funcionamiento del circuito; el siguiente paso es seleccionar componentes y diseñar la PCB. |
| - | - | Elaborar una lista de componentes con hoja de datos, enlace de compra y precio.<br>Definir el microcontrolador a utilizar. | Se buscan y comparan componentes, así como las características de distintos microcontroladores. | Se genera un archivo de Excel con los componentes, hojas de datos y precios. Además, se selecciona el ESP32 por sus características de comunicación. | Debido a la variedad de baterías, se requieren tensiones variables controladas desde la computadora; se propone utilizar un convertidor DC-DC con ajuste mediante un trimmer digital. |
| - | 3 horas | Definir los sensores de corriente y tensión. | Se realiza la búsqueda de sensores de corriente y tensión. | Se encuentran sensores tipo Arduino y sensores LEM (de alto costo). | Queda pendiente definir los sensores; se consultará con el profesor. |
| - | 5 horas | Diseñar la PCB en KiCad para presentarla al profesor. | Se dibuja el esquemático inicial de la PCB en KiCad y se genera una primera propuesta. | Se obtiene una PCB de 10 cm x 10 cm en la que se puede integrar el ESP32. | Queda pendiente presentarla al profesor. |
| 23/03/2026 | 2 horas | Presentar la propuesta de PCB al profesor y aclarar dudas del circuito y del sistema de control. | Se realiza una reunión en el laboratorio con el profesor, donde se aclaran dudas sobre la sección prealimentada del sistema de control y la fuente de tensión. | Se concluye que es necesario diseñar la caja del prototipo y utilizar medición de tensión con 4 cables. | Queda pendiente diseñar la caja e implementar medición tipo 4-wire. |
| 30/03/2026 | 4 horas | Diseñar un prototipo de la caja en TinkerCad para definir la disposición de la PCB. | Se diseña una caja en TinkerCad considerando las entradas, salidas y ubicación de la PCB. | Se obtiene un modelo de caja que puede utilizarse o modificarse para impresión. | Falta diseñar la tapa de la caja. |
| 09/04/2026 | 1 hora | Reunión de actualización con el profesor Mauricio. | Se actualiza al profesor sobre los avances del proyecto después de Semana Santa. | Se propone agregar LEDs para indicar carga y descarga, y se discuten las baterías a utilizar. | Se acuerda trabajar en las ecuaciones del controlador y realizar modificaciones a la PCB. Además, se definen nominalmente tres tipos de baterías. |



