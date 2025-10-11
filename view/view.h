/**
 * @brief Main view interface for Reversi game
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef VIEW_H
#define VIEW_H

#include "../model.h"

 // Main view functions
void initView();
void freeView();
void drawView(GameModel& model);
Move_t getMoveOnMousePointer();

// Menu functions
void drawMainMenu();
bool isMousePointerOverPlayBlackButton();
bool isMousePointerOverPlayWhiteButton();
bool isMousePointerOverAIEasyButton();
bool isMousePointerOverAINormalButton();
bool isMousePointerOverAIHardButton();
bool isMousePointerOverAIExtremeButton();

#endif // VIEW_H