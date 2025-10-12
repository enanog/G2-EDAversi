/**
 * @brief Main view implementation coordinating all rendering components
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "view.h"
#include "view_constants.h"
#include "ui_components.h"
#include "board_renderer.h"
#include "game_overlay.h"
#include "menu_system.h"
#include "settings_overlay.h"
#include "raylib.h"
#include <cmath>

 /**
  * @brief Initializes the game window and sets target FPS
  */
void initView() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, GAME_NAME);
    SetTargetFPS(60);
}

/**
 * @brief Closes the game window and cleans up resources
 */
void freeView() {
    CloseWindow();
}

/**
 * @brief Main view rendering function - coordinates all rendering components
 * @param model Current game model containing board state and game information
 * @param showSettings Whether to show the settings overlay
 * @param aiDifficulty Current AI difficulty setting
 * @param nodeLimit Current node limit for AI
 */
void drawView(GameModel& model, bool showSettings, const std::string& aiDifficulty, int nodeLimit, bool aiActivate) {
    BeginDrawing();
    ClearBackground(BEIGE);


    // Render board components
    drawBoard();
    drawValidMoves(model);
    drawBoardPieces(model);

    // Draw information panel
    drawScore("Black score: ", { INFO_CENTERED_X, INFO_WHITE_SCORE_Y }, getScore(model, PLAYER_BLACK));
    drawTimer({ INFO_CENTERED_X, INFO_WHITE_TIME_Y }, getTimer(model, PLAYER_BLACK));
    drawCenteredText({ INFO_CENTERED_X, INFO_TITLE_Y }, TITLE_FONT_SIZE, GAME_NAME);
    
    drawScore("White score: ", { INFO_CENTERED_X, INFO_BLACK_SCORE_Y }, getScore(model, PLAYER_WHITE));
    drawTimer({ INFO_CENTERED_X, INFO_BLACK_TIME_Y }, getTimer(model, PLAYER_WHITE));

    // Draw settings button
    drawSettingsButton();

    // Render overlays
    drawPassMessage(model);
    drawGameOverScreen(model);

    if (aiActivate)
    {
        Color aiColorDifficulty = LIGHTGRAY;
        if (aiDifficulty == "Easy") {
            aiColorDifficulty = GREEN;
        }
        else if (aiDifficulty == "Normal") {
            aiColorDifficulty = YELLOW;
        }
        else if (aiDifficulty == "Hard") {
            aiColorDifficulty = RED;
        }
        else if (aiDifficulty == "Extreme") {
            aiColorDifficulty = DARKPURPLE;
        }
        drawCenteredColoredText({ INFO_CENTERED_X, INFO_TITLE_Y + TITLE_FONT_SIZE * 0.8 }, SUBTITLE_FONT_SIZE, aiDifficulty, aiColorDifficulty);
    }

    // Settings overlay (drawn last to be on top)
    if (showSettings) {
        drawSettingsOverlay(aiDifficulty, nodeLimit);
    }

    EndDrawing();
}

/**
 * @brief Gets the board position under the mouse cursor
 * @return Board position (0-63) or MOVE_NONE if outside board
 */
Move_t getMoveOnMousePointer() {
    Vector2 mousePosition = GetMousePosition();

    int x = (int)floor((mousePosition.x - BOARD_X) / SQUARE_SIZE);
    int y = (int)floor((mousePosition.y - BOARD_Y) / SQUARE_SIZE);

    Move_t move = coordsToMove(x, y);

    if (isMoveInBounds(move))
        return move;
    else
        return MOVE_NONE;
}
