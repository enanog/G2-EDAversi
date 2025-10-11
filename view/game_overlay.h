/**
 * @brief Game overlay rendering (pass messages, game over screens)
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef GAME_OVERLAY_H
#define GAME_OVERLAY_H

#include "../model.h"

 // Overlay rendering
void drawPassMessage(const GameModel& model);
void drawGameOverScreen(const GameModel& model);

#endif // GAME_OVERLAY_H