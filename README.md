# EDAversi

## Integrantes del grupo y contribución al trabajo de cada integrante

* Frigerio, Dylan: Interfaz de usuario, sistema de menús, integración visual y threading
* Petersen, Alex: Motor del juego, sistema de movimientos válidos, lógica del tablero con bitboards
* Rosa Fernandez, Enzo: Desarrollo del motor de IA y de la dificultad extremo.
* Valenzuela, Agustín: Sistema de threading para IA, optimizaciones de rendimiento, integración de componentes

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


## Parte 3: Poda del árbol

Como se mencionó en el apartado anterior, el algoritmo minimax sin restricción de profundidad no se llega
a completar. Por lo tanto, se optó por aplicar dos técnicas distintas para optimizar el algoritmo.

Las dificultades que utilizan el algoritmo minimax son medio y difícil. Ambas dificultades utilizan el mismo algoritmo, pero en 
la dificultad media no se utiliza poda alfa-beta, mientras que en la dificultad difícil sí. Esto permite que la dificultad difícil 
pueda analizar más niveles en el árbol de decisiones en el mismo tiempo, haciendo que la IA juegue de manera más óptima.

En el caso de la dificultad media, solo se aplicó una poda por profundidad, es decir, se analiza un número fijo de
niveles en el árbol de decisiones. Esto limita la cantidad de jugadas que se consideran, haciendo que el algoritmo
sea más rápido, pero también menos preciso. Como métrica de evaluación, se utilizó la diferencia entre la cantidad
de fichas del jugador actual y la cantidad de fichas del oponente.

En el caso de la dificultad difícil, se aplicó la poda alfa-beta, que es una técnica que reduce el número de nodos
evaluados en el árbol de decisiones. La poda alfa-beta funciona manteniendo dos valores que representan los límites 
inferior y superior de la evaluación del nodo. Si en algún momento se encuentra un nodo que no puede mejorar la evaluación 
actual, se lo poda y no se evalúan sus hijos. La métrica de evaluación utilizada en este caso depende de cada espacio del
tablero obtenido, y se le asigna un valor según su posición estratégica. Por ejemplo, las esquinas del tablero tienen un valor alto
porque no pueden ser capturadas, mientras que las posiciones adyacentes a las esquinas tienen un valor bajo porque habilitan al 
oponente a capturar la esquina.

La complejidad computacional del algoritmo minimax sin poda es O(b^d), donde b es el factor de ramificación y d es la
profundidad del árbol. Mientras que la complejidad computacional del algoritmo minimax con poda alfa-beta en el peor caso 
sigue siendo O(b^d), pero en el mejor caso se reduce a O(b^(d/2)).

La mejora entre estos 2 algoritmos es significativa, ya que la poda alfa-beta permite analizar más niveles en el mismo tiempo.
En este caso, logramos que la dificultad normal analice 4 niveles del árbol de decisiones, mientras que la dificultad difícil
puede analizar 8 niveles en un tiempo similar. Esto es una gran diferencia al momento de tomar la decisión más óptima.


## Documentación adicional

Motores de referencia:

EDAX: Uno de los motores más potentes para Othello/Reversi, utilizado como referencia para técnicas avanzadas 
de búsqueda y evaluación. 
Link al motor EDAX: https://sensuikan1973.github.io/edax-reversi/

Investigación Académica:

Resolución de Reversi: Paper académico que aborda técnicas de resolución completa y algoritmos avanzados para 
el juego. 
Paper en arXiv: https://arxiv.org/pdf/2310.19387

Bases de Datos Profesionales:

Base WThor: ase de datos oficial utilizada en torneos profesionales de Othello, con miles de partidas para 
entrenamiento de sistemas de IA. 
Base WThor: https://www.ffothello.org/informatique/la-base-wthor/

Implementación de algoritmos de IA:
Algoritmo Minimax: https://www-instructables-com.translate.goog/Othello-Artificial-Intelligence/?_x_tr_sl=en&_x_tr_tl=es&_x_tr_hl=es&_x_tr_pto=tc
Uso de Claude/ChatGPT como Herramienta de Desarrollo de algoritmos, optimización de código, e interfaz gráfica.

## Bonus points

Además de los requisitos básicos, se implementaron las siguientes características adicionales al programa:

* Modos 1v1 y 1vIA: Se puede jugar contra otro jugador humano o contra la IA.

* Selección de dificultad de IA: Se pueden elegir entre 4 niveles de dificultad (fácil, medio, difícil y extremo).
  Esto también se puede hacer durante la partida desde el menú de configuración, pero cambia a partir del siguiente
  turno de la IA.

* Dificultad extremo: para esta dificultad se implementaron diversas técnicas para optimizar las decisiones
	- Algoritmo negamax con poda alfa-beta: reduce los nodos evaluados en el árbol de decisiones teniendo en cuenta
	  la simetría del juego.
	- Búsqueda incremental de profundidad: comienza evaluando con una profundidad baja al principio del juego, y la
	  va incrementando a medida que avanza el juego.
	- Tabla de transposición: almacena las evaluaciones de posiciones ya analizadas para evitar cálculos redundantes.
	- Libro de aperturas: utiliza un conjunto predefinido de movimientos iniciales para optimizar las primeras jugadas.
	
* Límite de nodos configurables: se puede establecer un límite en la cantidad de nodos que la IA puede evaluar por jugada.

* Thread separado para la IA: la IA se ejecuta en un hilo separado para mantener la interfaz de usuario receptiva.

* Optimización de bitmaps: se utilizaron operaciones a nivel de bits para mejorar la eficiencia en la generación de movimientos 
  válidos, actualización del tablero, y conteo de fichas.

* Mejoras en la interfaz gráfica:
	- Pantallas de victoria, empate y derrota personalizadas.
	- Indicación visual de los movimientos válidos y la cantidad de fichas que se capturarían.
	- Anuncio de que un jugador salteó su turno por no tener movimientos válidos.
	- Menú inicial para seleccionar el modo de juego y la dificultad de la IA.
	- Menú de configuración para ajustar el límite de nodos y otras opciones en partida.