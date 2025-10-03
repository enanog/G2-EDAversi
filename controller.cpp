/**
 * @brief Implements the Reversi game controller
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2023-2024
 */

#include <algorithm>

#include "raylib.h"

#include "ai.h"
#include "view.h"
#include "controller.h"

bool updateView(GameModel &model)
{
	if (WindowShouldClose())
		return false;

	if (model.gameOver)
	{
		if (IsMouseButtonPressed(0))
		{
			if (isMousePointerOverPlayBlackButton())
			{
				model.humanPlayer = PLAYER_BLACK;

				startModel(model);
			}
			else if (isMousePointerOverPlayWhiteButton())
			{
				model.humanPlayer = PLAYER_WHITE;

				startModel(model);
			}
		}
	}
	else if (model.currentPlayer == model.humanPlayer)
	{
		if (IsMouseButtonPressed(0))
		{
			// Human player
			Square_t square = getSquareOnMousePointer();
			int8_t n = GET_SQUARE_BIT_INDEX(square.x, square.y);

			if (isSquareValid(n))
			{
				Moves validMoves;
				getValidMoves(model, validMoves);
				// Play move if valid
				for (auto move : validMoves)
				{
					if ((square.x == move.x) &&
						(square.y == move.y))
						playMove(model, n);
				}
			}
		}
	}
	else
	{
		// AI player
		Square_t square = getBestMove(model);
		int8_t n = GET_SQUARE_BIT_INDEX(square.x, square.y);

		playMove(model, n);
	}

	if ((IsKeyDown(KEY_LEFT_ALT) ||
		IsKeyDown(KEY_RIGHT_ALT)) &&
		IsKeyPressed(KEY_ENTER))
		ToggleFullscreen();

	drawView(model);

	return true;
}