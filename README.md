# EDAversi

## Integrantes del grupo y contribución al trabajo de cada integrante

* Frigerio, Dylan: 
* Petersen, Alex:
* Rosa Fernandez, Enzo:
* Valenzuela, Agustín:

## Parte 1: Generación de movimientos válidos y algoritmo de jugada

El tablero de juego se representa como dos enteros de 64 bits, uno para cada jugador. Cada bit representa una casilla del tablero, 
y su valor indica si la casilla está vacía u ocupada por una ficha del jugador correspondiente.

Respecto a los movimientos válidos, se implementó el algoritmo de búsqueda de Kogge-Stone, que busca en las 8 direcciones posibles
desde la posición inicial para encontrar fichas del oponente que puedan ser capturadas. Este movimiento se propaga hasta que 
se encuentra una ficha del jugador actual o se sale del tablero. Si se encuentra una ficha del jugador actual, se considera que
el movimiento es válido. Al analizar cada movimiento, se van sumando las fichas del oponente que se capturarían en cada dirección.

Las pruebas realizadas para comprobar que los movimientos válidos y el algoritmo de jugada funcionan correctamente incluyen:
* Capturas en posiciones iniciales del tablero.
* Capturas en cada una de las 8 direcciones posibles.
* Capturas múltiples en un solo movimiento.
* Capturas en los bordes y esquinas del tablero.
* Situaciones donde no hay movimientos válidos disponibles.
* Situaciones donde un jugador tiene movimientos válidos y el otro no.
* Situaciónes donde no hay movimientos válidos para ningún jugador.

Para las comparaciones con un juego y pruebas de referencia, se utilizó la página:
https://playpager.com/juego-reversi/
 
Además, se verificó que el conteo de fichas capturadas sea correcto en cada caso, y que el estado del tablero se actualice correctamente
después de cada jugada, sin superponer fichas ni dejar espacios vacíos incorrectamente.

## Parte 2: Implementación del motor de IA

Para el motor de IA, se implementaron 4 dificultades distintas contra las cuales se puede jugar: fácil, medio, difícil y extremo.

El algoritmo minimax consiste en seleccionar el movimiento que maximiza la ganancia de puntos para el jugador actual, 
asumiendo que el oponente también juega de manera óptima.

Al probar el algoritmo minimax sin restricciones, se encontró que el proceso nunca acababa. Esto se debe
a que el árbol de decisiones crece exponencialmente con cada nivel adicional. Por lo que, sin un límite
de profundidad, el algoritmo intenta explorar todas posibles jugadas del Reversi, lo cual se aproxima
a 10 elevado a la 28 jugadas posibles.


[Enumera aquí las consideraciones que tomaste a la hora de implementar el algoritmo minimax.]

## Parte 3: Poda del árbol

Las dificultades que utilizan el algoritmo minimax son medio y difícil. Ambas dificultades utilizan el mismo algoritmo, pero en 
la dificultad media no se utiliza poda alfa-beta, mientras que en la dificultad difícil sí. Esto permite que la dificultad difícil 
pueda analizar más niveles en el árbol de decisiones en el mismo tiempo, haciendo que la IA juegue de manera más óptima.

Como se mencionó en el apartado anterior, el algoritmo minimax sin restricción de profundidad no se llega
a completar. Por lo tanto, se 

(Las dificultades fácil y extremo se explican a detalle en la sección de bonus points.)
[Justifica por qué el algoritmo minimax de la parte anterior no se completa. Consejo: determina la complejidad computacional.]

## Documentación adicional



[Aquí.]

## Bonus points

[Aquí.]
