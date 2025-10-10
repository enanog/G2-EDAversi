/**
 * @brief Implements the Reversi game AI
 * @author Marc S. Ressl
 * @modified:
 *			Agustin Valenzuela,
 *			Alex Petersen,
 *			Dylan Frigerio,
 *			Enzo Fernadez Rosas
 * 
 * @copyright Copyright (c) 2023-2024
 */

#ifndef AI_H
#define AI_H

#include "model.h"

 /**
  * @brief Returns the best move for a certain position.
  * @param model The game model
  * @return The best move (Move_t 0-63), or MOVE_NONE if no valid moves
  */
Move_t getBestMove(GameModel& model);

#endif