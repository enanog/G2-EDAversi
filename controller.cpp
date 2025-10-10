/**
 * @brief Implements the Reversi game controller
 * @author Marc S. Ressl
 * @modified:
 *			Agustin Valenzuela,
 *			Alex Petersen,
 *			Dylan Frigerio,
 *			Enzo Fernadez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include <algorithm>

#include "raylib.h"

#include "ai.h"
#include "view.h"
#include "controller.h"

#include "controller.h"

#include <algorithm>

#include "raylib.h"
#include "ai.h"
#include "view.h"

bool updateView(GameModel& model) {
    if (WindowShouldClose())
        return false;

    if (model.gameOver) {
        // Pantalla de inicio/fin de juego
        if (IsMouseButtonPressed(0)) {
            if (isMousePointerOverPlayBlackButton()) {
                model.humanPlayer = PLAYER_BLACK;
                startModel(model);
            }
            else if (isMousePointerOverPlayWhiteButton()) {
                model.humanPlayer = PLAYER_WHITE;
                startModel(model);
            }
        }
    }
    else if (model.currentPlayer == model.humanPlayer) {
        // Turno del humano
        if (IsMouseButtonPressed(0)) {
            Move_t move = getMoveOnMousePointer();

            if (move != MOVE_NONE) {
                // Verificar si el movimiento es válido
                MoveList validMoves;
                getValidMoves(model, validMoves);

                // Buscar el movimiento en la lista de movimientos válidos
                auto it = std::find(validMoves.begin(), validMoves.end(), move);
                if (it != validMoves.end()) {
                    playMove(model, move);
                }
            }
        }
    }
    else {
        // Turno de la AI
        Move_t move = getBestMove(model);

        if (move != MOVE_NONE) {
            playMove(model, move);
        }
    }

    // Toggle fullscreen con Alt+Enter
    if ((IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) &&
        IsKeyPressed(KEY_ENTER)) {
        ToggleFullscreen();
    }

    drawView(model);

    return true;
}