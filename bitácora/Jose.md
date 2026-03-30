# Bitácora de trabajo 

Jose Manuel Solis
C27645 


| Fecha       | Horas   | Objetivos | Descripción | Resultados | Comentario |
|:-----------:|:-------:|-----------|-------------|------------|------------|
| - | - | Exponer las ideas del grupo <br> Dibujar un modelo de lo que se quiere.| Se diseña un borrador del protoripo final que se quiere implementar. | Se dibuja un esquematico con las partes del prototipo. | Queda pendiente definir si utilizar un ESP-32 o un C2000.|
| - | - | Diseñar un circuito y un lazo de control que se encargue de ciclar la bateria. <br> Simular el circuito y verficiar que puede cargar y descargar la bateria con la topología utilizada. | Se diseño el circuito del ciclador y se simuló en PSpice. | La simulación del circuito cumplio con cargar y descargar la bateria cuando se le da la señal al amplificador operacional por medio del sistema de control. | Falta hacer los calculos de la señal de control para verificar que el circuito si cumple con las ecuaciones de carga y descarga.|
| - | - | Calcular la acción de control de la malla de carga y descarga. | Se hicierón los calculos correspondientes para identificar la acción de control de cada malla. | La acción de control dio como se esperaba, el circuito de carga positivo y el de descarga negativo. | Se aseguró que el circuito funciona, por lo que lo proximo es buscar los componentes y diseñar la PCB.|
| - | - | Hacer una lista de los componentes a utilizar, en donde se tenga la hoja de datos, el link de compra y el precio. <br>Definir el microcontrolador que se va utilizar. | Se buscarón y compararón los componentes. Se buscarón las caracterisiticas de los microcontroladores. | Se generón un archivo de exel con los compoentes que se van a utilizar, hoas de datos y precio. Ademas se escogio el microcontrolador ESP-32 por sus caracterisiticas de comunicación. | Al utilizar diferentes baterias se requieren diferentes tensiones controladas por medio de la computadora, por eso se requiere un convertidor DC-DC variables que se va a hacer comprando un convertidor DC-DC normal y colocandole un trimmer digital en el trimmer que ya viene.  |
| - | 3 Horas | Definir los sesnores de corriente y tensión. | Búsqueda de sensores de corriente y tensión. | Se encontraron sensores de arduino como lo que se tenian y sensores LEM muy caros. | Queda pendiente definir los sensores, para eso se va hablar con el profesor. |
| - | 5 horas | Diseñar la PCB en KiCad para presentarsela al profesor. | Se dibuja el esquemático inicial de la PCB en KiCad y se genera la primera propuesta para el profesor. | Se obtuvo una PBC de 10cm x 10cm en la que se puede poner el ESP32.| Queda pendiente presentarsela al profesor. |
| 23/3/2025 | 2 Horas |Presentar la propuesta de la PCB al profesor y despejar dudas del circuito y sistema de control. | Se tuvo una reunión en el laboratorio con el profesor, se aclararón las dudas del grupo del sistema de control en la sección prealimentada y del circuito en la fuente de tensión. | Se llego a la conclusión de que se ocupa diseñar la caja que va a contener la electronica para terminar la PCB y se requiere diseñar los puertos para medir la tensión de la bateria por 4 cables. | Queda pendiente diseñar la caja del prototipo e implementar 4 wire para medir la tensión.|




