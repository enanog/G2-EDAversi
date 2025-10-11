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

#include <cmath>
#include <string>

#include "controller.h"
#include "raylib.h"
#include "view.h"

#define GAME_NAME "EDAversi"

 // Window configuration
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

// Board rendering constants
#define SQUARE_SIZE 80
#define SQUARE_PADDING 1.5F
#define SQUARE_CONTENT_OFFSET (SQUARE_PADDING)
#define SQUARE_CONTENT_SIZE (SQUARE_SIZE - 2 * SQUARE_PADDING)

#define PIECE_CENTER (SQUARE_SIZE / 2)
#define PIECE_RADIUS (SQUARE_SIZE * 80 / 100 / 2)

#define BOARD_X 40
#define BOARD_Y 40
#define BOARD_CONTENT_SIZE (BOARD_SIZE * SQUARE_SIZE)

#define OUTERBORDER_X (BOARD_X - OUTERBORDER_PADDING)
#define OUTERBORDER_Y (BOARD_Y - OUTERBORDER_PADDING)
#define OUTERBORDER_PADDING 40
#define OUTERBORDER_WIDTH 10
#define OUTERBORDER_SIZE (BOARD_CONTENT_SIZE + 2 * OUTERBORDER_PADDING)

// Text rendering constants
#define TITLE_FONT_SIZE 72
#define SUBTITLE_FONT_SIZE 36

// Info panel positioning
#define INFO_CENTERED_X (OUTERBORDER_SIZE + (WINDOW_WIDTH - OUTERBORDER_SIZE) / 2)

#define INFO_TITLE_Y (WINDOW_HEIGHT / 2)

#define INFO_WHITE_SCORE_Y (WINDOW_HEIGHT * 1 / 4 - SUBTITLE_FONT_SIZE / 2)
#define INFO_WHITE_TIME_Y (WINDOW_HEIGHT * 1 / 4 + SUBTITLE_FONT_SIZE / 2)

#define INFO_BLACK_SCORE_Y (WINDOW_HEIGHT * 3 / 4 - SUBTITLE_FONT_SIZE / 2)
#define INFO_BLACK_TIME_Y (WINDOW_HEIGHT * 3 / 4 + SUBTITLE_FONT_SIZE / 2)

// Button configuration
#define INFO_BUTTON_WIDTH 280
#define INFO_BUTTON_HEIGHT 64

#define INFO_PLAYBLACK_BUTTON_X INFO_CENTERED_X
#define INFO_PLAYBLACK_BUTTON_Y (WINDOW_HEIGHT * 1 / 8)

#define INFO_PLAYWHITE_BUTTON_X INFO_CENTERED_X
#define INFO_PLAYWHITE_BUTTON_Y (WINDOW_HEIGHT * 7 / 8)

// AI difficulty selection button positions
#define AI_BUTTON_X (WINDOW_WIDTH * 1 / 2)
#define AI_EASY_BUTTON_Y (WINDOW_HEIGHT * 3 / 8)
#define AI_NORMAL_BUTTON_Y (WINDOW_HEIGHT * 4 / 8)
#define AI_HARD_BUTTON_Y (WINDOW_HEIGHT * 5 / 8)
#define AI_EXTREME_BUTTON_Y (WINDOW_HEIGHT * 6 / 8)

void initView() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, GAME_NAME);
    SetTargetFPS(60);
}

void freeView() {
    CloseWindow();
}

/**
 * @brief Draws text centered around specified position
 */
static void drawCenteredText(Vector2 position, int fontSize, std::string text) {
    DrawText(text.c_str(),
        (int)position.x - MeasureText(text.c_str(), fontSize) / 2,
        (int)position.y - fontSize / 2,
        fontSize,
        BROWN);
}

/**
 * @brief Draws player score with label
 */
static void drawScore(std::string label, Vector2 position, int score) {
    std::string displayText = label + std::to_string(score);
    drawCenteredText(position, SUBTITLE_FONT_SIZE, displayText);
}

/**
 * @brief Draws formatted timer (MM:SS)
 */
static void drawTimer(Vector2 position, double time) {
    int totalSeconds = (int)time;
    int seconds = totalSeconds % 60;
    int minutes = totalSeconds / 60;

    std::string timeString;
    if (minutes < 10)
        timeString.append("0");
    timeString.append(std::to_string(minutes));
    timeString.append(":");
    if (seconds < 10)
        timeString.append("0");
    timeString.append(std::to_string(seconds));

    drawCenteredText(position, SUBTITLE_FONT_SIZE, timeString);
}

/**
 * @brief Draws a rectangular button with centered text
 */
static void drawButton(Vector2 position, std::string label, Color backgroundColor) {
    DrawRectangle(position.x - INFO_BUTTON_WIDTH / 2,
        position.y - INFO_BUTTON_HEIGHT / 2,
        INFO_BUTTON_WIDTH,
        INFO_BUTTON_HEIGHT,
        backgroundColor);

    drawCenteredText({ position.x, position.y }, SUBTITLE_FONT_SIZE, label.c_str());
}

/**
 * @brief Checks if mouse pointer is within button bounds
 */
static bool isMousePointerOverButton(Vector2 position) {
    Vector2 mousePosition = GetMousePosition();

    return ((mousePosition.x >= (position.x - INFO_BUTTON_WIDTH / 2)) &&
        (mousePosition.x < (position.x + INFO_BUTTON_WIDTH / 2)) &&
        (mousePosition.y >= (position.y - INFO_BUTTON_HEIGHT / 2)) &&
        (mousePosition.y < (position.y + INFO_BUTTON_HEIGHT / 2)));
}

void drawView(GameModel& model) {
    BeginDrawing();

    ClearBackground(BEIGE);

    // Draw outer board border
    DrawRectangle(OUTERBORDER_X, OUTERBORDER_Y, OUTERBORDER_SIZE, OUTERBORDER_SIZE, BLACK);

    // Draw board squares
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            Vector2 position = { BOARD_X + (float)x * SQUARE_SIZE,
                                BOARD_Y + (float)y * SQUARE_SIZE };

            DrawRectangleRounded({ position.x + SQUARE_CONTENT_OFFSET,
                                  position.y + SQUARE_CONTENT_OFFSET,
                                  SQUARE_CONTENT_SIZE,
                                  SQUARE_CONTENT_SIZE },
                0.2F,
                6,
                DARKGREEN);
        }
    }

    // Highlight valid moves during active gameplay
    if (!model.gameOver) {
        MoveList validMoves;
        getValidMoves(model, validMoves);

        // Obtener bitboards del jugador actual
        uint64_t player = getPlayerBitboard(model.board, getCurrentPlayer(model));
        uint64_t opponent = getOpponentBitboard(model.board, getCurrentPlayer(model));
		Color highlightColor = (getCurrentPlayer(model) == PLAYER_BLACK) ? BLACK : WHITE;

        for (Move_t move : validMoves) {
            int x = getMoveX(move);
            int y = getMoveY(move);

            // Calcular centro del círculo
            int centerX = (int)(BOARD_X + (float)x * SQUARE_SIZE) + PIECE_CENTER;
            int centerY = (int)(BOARD_Y + (float)y * SQUARE_SIZE) + PIECE_CENTER;

            // Dibujar punto rojo
            DrawCircle(centerX, centerY, PIECE_RADIUS, highlightColor);
            DrawCircle(centerX, centerY, PIECE_RADIUS * 0.8, DARKGREEN);

            // Calcular cantidad de fichas que se voltearían
            uint64_t flips = calculateFlips(player, opponent, move);
            int flipCount = countBits(flips);

            // Dibujar el número de flips
            // Ajustar posición según cantidad de dígitos para centrar mejor
            const char* text = TextFormat("%d", flipCount);
            int fontSize = 30;
            int textWidth = MeasureText(text, fontSize);

            // Dibujar texto con borde negro para mejor legibilidad
            DrawText(text, centerX, centerY, fontSize, highlightColor);
        }
    }

    // Draw game pieces
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            Move_t move = coordsToMove(x, y);
            PieceState_t piece = getBoardPiece(model, move);

            if (piece != STATE_EMPTY) {
                Vector2 position = { BOARD_X + (float)x * SQUARE_SIZE,
                                    BOARD_Y + (float)y * SQUARE_SIZE };
                DrawCircle((int)position.x + PIECE_CENTER,
                    (int)position.y + PIECE_CENTER,
                    PIECE_RADIUS,
                    (piece == STATE_WHITE) ? WHITE : BLACK);
            }
        }
    }

    // Draw information panel
    drawScore("Black score: ", { INFO_CENTERED_X, INFO_WHITE_SCORE_Y },
        getScore(model, PLAYER_BLACK));
    drawTimer({ INFO_CENTERED_X, INFO_WHITE_TIME_Y }, getTimer(model, PLAYER_BLACK));
    drawCenteredText({ INFO_CENTERED_X, INFO_TITLE_Y }, TITLE_FONT_SIZE, GAME_NAME);
    drawScore("White score: ", { INFO_CENTERED_X, INFO_BLACK_SCORE_Y },
        getScore(model, PLAYER_WHITE));
    drawTimer({ INFO_CENTERED_X, INFO_BLACK_TIME_Y }, getTimer(model, PLAYER_WHITE));

    // Show game over buttons
    if (model.gameOver) {
        drawButton({ INFO_PLAYBLACK_BUTTON_X, INFO_PLAYBLACK_BUTTON_Y }, "Play black", BLACK);
        drawButton({ INFO_PLAYWHITE_BUTTON_X, INFO_PLAYWHITE_BUTTON_Y }, "Play white", WHITE);
    }

    EndDrawing();
}

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

bool isMousePointerOverPlayBlackButton() {
    return isMousePointerOverButton({ INFO_PLAYBLACK_BUTTON_X, INFO_PLAYBLACK_BUTTON_Y });
}

bool isMousePointerOverPlayWhiteButton() {
    return isMousePointerOverButton({ INFO_PLAYWHITE_BUTTON_X, INFO_PLAYWHITE_BUTTON_Y });
}

// AI difficulty button handlers
bool isMousePointerOverAIEasyButton() {
    return isMousePointerOverButton({ AI_BUTTON_X, AI_EASY_BUTTON_Y });
}

bool isMousePointerOverAINormalButton() {
    return isMousePointerOverButton({ AI_BUTTON_X, AI_NORMAL_BUTTON_Y });
}

bool isMousePointerOverAIHardButton() {
    return isMousePointerOverButton({ AI_BUTTON_X, AI_HARD_BUTTON_Y });
}

bool isMousePointerOverAIExtremeButton() {
    return isMousePointerOverButton({ AI_BUTTON_X, AI_EXTREME_BUTTON_Y });
}

/**
 * @brief Draws AI difficulty selection buttons
 */
void drawAIDifficultyButtons() {
    drawButton({ AI_BUTTON_X, AI_EASY_BUTTON_Y }, "Easy AI", LIGHTGRAY);
    drawButton({ AI_BUTTON_X, AI_NORMAL_BUTTON_Y }, "Normal AI", GRAY);
    drawButton({ AI_BUTTON_X, AI_HARD_BUTTON_Y }, "Hard AI", DARKGRAY);
    drawButton({ AI_BUTTON_X, AI_EXTREME_BUTTON_Y }, "Extreme AI", MAROON);
}

/**
 * @brief Draws main menu screen with title and difficulty selection
 */
void drawMainMenu() {
    BeginDrawing();
    ClearBackground(BEIGE);

    // Title section
    drawCenteredText({ INFO_CENTERED_X, (float)WINDOW_HEIGHT * 1 / 6 }, TITLE_FONT_SIZE, GAME_NAME);
    drawCenteredText({ INFO_CENTERED_X, (float)WINDOW_HEIGHT * 2 / 6 }, SUBTITLE_FONT_SIZE, "Select AI difficulty");

    // Difficulty selection buttons
    drawAIDifficultyButtons();

    EndDrawing();
}