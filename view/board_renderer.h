/**
 * @brief Board and piece rendering functionality
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef BOARD_RENDERER_H
#define BOARD_RENDERER_H

#include "../model.h"

 // Board rendering
void drawBoard();
void drawBoardPieces(const GameModel& model);
void drawValidMoves(const GameModel& model);

#endif // BOARD_RENDERER_H