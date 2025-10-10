#ifndef VIEW_H
#define VIEW_H

#include "model.h"

/**
 * @brief Initializes a game view.
 */
void initView();

/**
 * @brief Frees the game view.
 */
void freeView();

/**
 * @brief Draws the game view.
 * @param model The game model.
 */
void drawView(GameModel& model);

/**
 * @brief Returns the move (square) under the mouse pointer.
 * @return Move_t (0-63) or MOVE_NONE if outside board
 */
Move_t getMoveOnMousePointer();

/**
 * @brief Indicates whether the mouse pointer is over the "Play black" button.
 * @return true or false.
 */
bool isMousePointerOverPlayBlackButton();

/**
 * @brief Indicates whether the mouse pointer is over the "Play white" button.
 * @return true or false.
 */
bool isMousePointerOverPlayWhiteButton();

#endif // VIEW_H