/**
 * @brief Implements the Reversi game view with difficulty selection menu
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

#include "model.h"

 /**
  * @brief Initializes the game view and window
  */
void initView();

/**
 * @brief Frees the game view resources
 */
void freeView();

/**
 * @brief Draws the main game view
 * @param model The game model containing current state
 */
void drawView(GameModel& model);

/**
 * @brief Gets the board position under mouse cursor
 * @return Move_t (0-63) or MOVE_NONE if outside board
 */
Move_t getMoveOnMousePointer();

/**
 * @brief Checks if mouse is over the "Play black" button
 */
bool isMousePointerOverPlayBlackButton();

/**
 * @brief Checks if mouse is over the "Play white" button
 */
bool isMousePointerOverPlayWhiteButton();

/**
 * @brief Draws the main menu with difficulty selection
 * Handles its own BeginDrawing/EndDrawing calls
 */
void drawMainMenu();

// Difficulty selection button handlers
bool isMousePointerOverAIEasyButton();
bool isMousePointerOverAINormalButton();
bool isMousePointerOverAIHardButton();
bool isMousePointerOverAIExtremeButton();

#endif // VIEW_H