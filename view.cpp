/**
 * @brief Implements the Reversi game view rendering and input handling
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
#include "model.h"

#define GAME_NAME "EDAversi"

 // Window configuration constants
static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;

// Board rendering constants
static const int SQUARE_SIZE = 80;
static const float SQUARE_PADDING = 1.5F;
static const float SQUARE_CONTENT_OFFSET = SQUARE_PADDING;
static const float SQUARE_CONTENT_SIZE = SQUARE_SIZE - 2 * SQUARE_PADDING;
static const int PIECE_CENTER = SQUARE_SIZE / 2;
static const int PIECE_RADIUS = SQUARE_SIZE * 80 / 100 / 2;
static const int BOARD_X = 40;
static const int BOARD_Y = 40;
static const int BOARD_CONTENT_SIZE = BOARD_SIZE * SQUARE_SIZE;
static const int OUTERBORDER_PADDING = 40;
static const int OUTERBORDER_WIDTH = 10;
static const int OUTERBORDER_SIZE = BOARD_CONTENT_SIZE + 2 * OUTERBORDER_PADDING;

// Text rendering constants
static const int TITLE_FONT_SIZE = 72;
static const int SUBTITLE_FONT_SIZE = 36;

// Info panel positioning constants
static const int INFO_CENTERED_X = OUTERBORDER_SIZE + (WINDOW_WIDTH - OUTERBORDER_SIZE) / 2;
static const int INFO_TITLE_Y = WINDOW_HEIGHT / 2;
static const int INFO_WHITE_SCORE_Y = WINDOW_HEIGHT * 1 / 4 - SUBTITLE_FONT_SIZE / 2;
static const int INFO_WHITE_TIME_Y = WINDOW_HEIGHT * 1 / 4 + SUBTITLE_FONT_SIZE / 2;
static const int INFO_BLACK_SCORE_Y = WINDOW_HEIGHT * 3 / 4 - SUBTITLE_FONT_SIZE / 2;
static const int INFO_BLACK_TIME_Y = WINDOW_HEIGHT * 3 / 4 + SUBTITLE_FONT_SIZE / 2;

// Button configuration constants
static const int INFO_BUTTON_WIDTH = 280;
static const int INFO_BUTTON_HEIGHT = 64;
static const int INFO_PLAYBLACK_BUTTON_X = INFO_CENTERED_X;
static const int INFO_PLAYBLACK_BUTTON_Y = WINDOW_HEIGHT * 1 / 8;
static const int INFO_PLAYWHITE_BUTTON_X = INFO_CENTERED_X;
static const int INFO_PLAYWHITE_BUTTON_Y = WINDOW_HEIGHT * 7 / 8;

// AI difficulty selection button positions
static const int AI_BUTTON_X = WINDOW_WIDTH * 1 / 2;
static const int AI_EASY_BUTTON_Y = WINDOW_HEIGHT * 3 / 8;
static const int AI_NORMAL_BUTTON_Y = WINDOW_HEIGHT * 4 / 8;
static const int AI_HARD_BUTTON_Y = WINDOW_HEIGHT * 5 / 8;
static const int AI_EXTREME_BUTTON_Y = WINDOW_HEIGHT * 6 / 8;

/**
 * @brief Initializes the game window and sets target FPS
 */
void initView() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, GAME_NAME);
    SetTargetFPS(60);
}

/**
 * @brief Closes the game window and frees resources
 */
void freeView() {
    CloseWindow();
}

/**
 * @brief Draws text centered around specified position using brown color
 * @param position Center position of the text
 * @param fontSize Size of the font
 * @param text Text to draw
 */
static void drawCenteredText(Vector2 position, int fontSize, const std::string& text) {
    int textWidth = MeasureText(text.c_str(), fontSize);
    DrawText(text.c_str(),
        (int)position.x - textWidth / 2,
        (int)position.y - fontSize / 2,
        fontSize,
        BROWN);
}

/**
 * @brief Draws player score with label at specified position
 * @param label Score label prefix
 * @param position Center position
 * @param score Score value to display
 */
static void drawScore(const std::string& label, Vector2 position, int score) {
    std::string displayText = label + std::to_string(score);
    drawCenteredText(position, SUBTITLE_FONT_SIZE, displayText);
}

/**
 * @brief Draws formatted timer in MM:SS format
 * @param position Center position of the timer display
 * @param time Total time in seconds
 */
static void drawTimer(Vector2 position, double time) {
    int totalSeconds = (int)time;
    int seconds = totalSeconds % 60;
    int minutes = totalSeconds / 60;

    std::string timeString;
    if (minutes < 10) timeString.append("0");
    timeString.append(std::to_string(minutes));
    timeString.append(":");
    if (seconds < 10) timeString.append("0");
    timeString.append(std::to_string(seconds));

    drawCenteredText(position, SUBTITLE_FONT_SIZE, timeString);
}

/**
 * @brief Draws a rectangular button with centered text
 * @param position Center position of the button
 * @param label Button text
 * @param backgroundColor Button background color
 */
static void drawButton(Vector2 position, const std::string& label, Color backgroundColor) {
    DrawRectangle(position.x - INFO_BUTTON_WIDTH / 2,
        position.y - INFO_BUTTON_HEIGHT / 2,
        INFO_BUTTON_WIDTH,
        INFO_BUTTON_HEIGHT,
        backgroundColor);
    drawCenteredText(position, SUBTITLE_FONT_SIZE, label);
}

/**
 * @brief Checks if mouse pointer is within specified button bounds
 * @param position Center position of the button
 * @return True if mouse is over the button
 */
static bool isMousePointerOverButton(Vector2 position) {
    Vector2 mousePosition = GetMousePosition();
    return (mousePosition.x >= (position.x - INFO_BUTTON_WIDTH / 2) &&
        mousePosition.x < (position.x + INFO_BUTTON_WIDTH / 2) &&
        mousePosition.y >= (position.y - INFO_BUTTON_HEIGHT / 2) &&
        mousePosition.y < (position.y + INFO_BUTTON_HEIGHT / 2));
}

/**
 * @brief Draws centered text with custom color
 * @param position Center position
 * @param fontSize Font size
 * @param text Text content
 * @param color Text color
 */
static void drawCenteredColoredText(Vector2 position, int fontSize, const std::string& text, Color color) {
    int textWidth = MeasureText(text.c_str(), fontSize);
    DrawText(text.c_str(),
        (int)position.x - textWidth / 2,
        (int)position.y - fontSize / 2,
        fontSize,
        color);
}

/**
 * @brief Renders the complete game view including board, pieces, scores, and UI elements
 * @param model Current game model state
 */
void drawView(GameModel& model) {
    BeginDrawing();
    ClearBackground(BEIGE);

    // Draw outer board border
    const int outerBorderX = BOARD_X - OUTERBORDER_PADDING;
    const int outerBorderY = BOARD_Y - OUTERBORDER_PADDING;
    DrawRectangle(outerBorderX, outerBorderY, OUTERBORDER_SIZE, OUTERBORDER_SIZE, BLACK);

    // Draw board squares with rounded corners
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            Vector2 position = { BOARD_X + (float)x * SQUARE_SIZE,
                                BOARD_Y + (float)y * SQUARE_SIZE };

            Rectangle squareRect = {
                position.x + SQUARE_CONTENT_OFFSET,
                position.y + SQUARE_CONTENT_OFFSET,
                SQUARE_CONTENT_SIZE,
                SQUARE_CONTENT_SIZE
            };
            DrawRectangleRounded(squareRect, 0.2F, 6, DARKGREEN);
        }
    }

    // Highlight valid moves for current player (excluding pass message display)
    if (!model.gameOver && !model.showPassMessage) {
        MoveList validMoves;
        getValidMoves(model, validMoves);

        PlayerColor_t currentPlayer = getCurrentPlayer(model);
        uint64_t player = getPlayerBitboard(model.board, currentPlayer);
        uint64_t opponent = getOpponentBitboard(model.board, currentPlayer);
        Color highlightColor = (currentPlayer == PLAYER_BLACK) ? BLACK : WHITE;

        for (Move_t move : validMoves) {
            int x = getMoveX(move);
            int y = getMoveY(move);

            // Calculate circle center
            int centerX = BOARD_X + (int)(x * SQUARE_SIZE) + PIECE_CENTER;
            int centerY = BOARD_Y + (int)(y * SQUARE_SIZE) + PIECE_CENTER;

            // Draw highlight circle with inner border
            DrawCircle(centerX, centerY, PIECE_RADIUS, highlightColor);
            DrawCircle(centerX, centerY, PIECE_RADIUS * 0.8, DARKGREEN);

            // Display flip count
            uint64_t flips = calculateFlips(player, opponent, move);
            int flipCount = countBits(flips);
            const char* text = TextFormat("%d", flipCount);
            int fontSize = 30;
            int textWidth = MeasureText(text, fontSize);
            DrawText(text, centerX - textWidth / 2, centerY - fontSize / 2, fontSize, highlightColor);
        }
    }

    // Draw game pieces on board
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            Move_t move = coordsToMove(x, y);
            PieceState_t piece = getBoardPiece(model, move);

            if (piece != STATE_EMPTY) {
                Vector2 position = { BOARD_X + (float)x * SQUARE_SIZE,
                                    BOARD_Y + (float)y * SQUARE_SIZE };
                Color pieceColor = (piece == STATE_WHITE) ? WHITE : BLACK;
                DrawCircle((int)position.x + PIECE_CENTER,
                    (int)position.y + PIECE_CENTER,
                    PIECE_RADIUS,
                    pieceColor);
            }
        }
    }

    // Draw player information panel (scores and timers)
    drawScore("Black score: ", { INFO_CENTERED_X, INFO_BLACK_SCORE_Y }, getScore(model, PLAYER_BLACK));
    drawTimer({ INFO_CENTERED_X, INFO_BLACK_TIME_Y }, getTimer(model, PLAYER_BLACK));
    drawCenteredText({ INFO_CENTERED_X, INFO_TITLE_Y }, TITLE_FONT_SIZE, GAME_NAME);
    drawScore("White score: ", { INFO_CENTERED_X, INFO_WHITE_SCORE_Y }, getScore(model, PLAYER_WHITE));
    drawTimer({ INFO_CENTERED_X, INFO_WHITE_TIME_Y }, getTimer(model, PLAYER_WHITE));

    // Display pass turn notification overlay
    if (model.showPassMessage) {
        std::string passMessage = (model.passedPlayer == PLAYER_BLACK) ? "BLACK PASSES TURN" : "WHITE PASSES TURN";
        Color messageColor = (model.passedPlayer == PLAYER_BLACK) ? BLACK : WHITE;
        Color bgColor = (model.passedPlayer == PLAYER_BLACK) ? WHITE : BLACK;

        // Semi-transparent background overlay
        DrawRectangle(BOARD_X,
            BOARD_Y + BOARD_CONTENT_SIZE / 2 - 50,
            BOARD_CONTENT_SIZE,
            100,
            Fade(bgColor, 0.85f));

        // Centered message text
        int messageWidth = MeasureText(passMessage.c_str(), SUBTITLE_FONT_SIZE);
        DrawText(passMessage.c_str(),
            BOARD_X + BOARD_CONTENT_SIZE / 2 - messageWidth / 2,
            BOARD_Y + BOARD_CONTENT_SIZE / 2 - SUBTITLE_FONT_SIZE / 2,
            SUBTITLE_FONT_SIZE,
            messageColor);
    }

    // Game over overlay with restart buttons and results
    if (model.gameOver) {
        drawButton({ INFO_PLAYBLACK_BUTTON_X, INFO_PLAYBLACK_BUTTON_Y }, "Play black", BLACK);
        drawButton({ INFO_PLAYWHITE_BUTTON_X, INFO_PLAYWHITE_BUTTON_Y }, "Play white", WHITE);

        int blackScore = getScore(model, PLAYER_BLACK);
        int whiteScore = getScore(model, PLAYER_WHITE);

        if (!blackScore && !whiteScore) {
            EndDrawing();
            return;
        }

        // Determine winner and prepare display elements
        std::string titleText, diffText;
        Color titleColor, bgColor;

        if (blackScore > whiteScore) {
            titleText = "BLACK WINS";
            titleColor = BLACK;
            bgColor = Fade(WHITE, 0.75f);
            diffText = "Difference: " + std::to_string(blackScore - whiteScore);
        }
        else if (whiteScore > blackScore) {
            titleText = "WHITE WINS";
            titleColor = WHITE;
            bgColor = Fade(BLACK, 0.75f);
            diffText = "Difference: " + std::to_string(whiteScore - blackScore);
        }
        else {
            titleText = "DRAW";
            titleColor = BEIGE;
            bgColor = Fade(DARKGRAY, 0.75f);
            diffText = "Equal score: " + std::to_string(blackScore) + " - " + std::to_string(whiteScore);
        }

        // Draw centered overlay with results
        int overlayWidth = BOARD_CONTENT_SIZE;
        int overlayHeight = SQUARE_SIZE * 2.5;
        int overlayX = BOARD_X;
        int overlayY = BOARD_Y + BOARD_CONTENT_SIZE / 2 - overlayHeight / 2;

        DrawRectangle(overlayX, overlayY, overlayWidth, overlayHeight, bgColor);
        DrawRectangleLines(overlayX, overlayY, overlayWidth, overlayHeight, titleColor);

        Vector2 overlayCenter = { (float)(overlayX + overlayWidth / 2), (float)(overlayY + overlayHeight / 2) };
        drawCenteredColoredText({ overlayCenter.x, overlayCenter.y - 20 }, TITLE_FONT_SIZE, titleText, titleColor);
        drawCenteredColoredText({ overlayCenter.x, overlayCenter.y + 28 }, SUBTITLE_FONT_SIZE, diffText, titleColor);
    }

    EndDrawing();
}

/**
 * @brief Converts mouse position to board square index
 * @return Move_t position (0-63) or MOVE_NONE if outside board
 */
Move_t getMoveOnMousePointer() {
    Vector2 mousePosition = GetMousePosition();
    int x = (int)floor((mousePosition.x - BOARD_X) / SQUARE_SIZE);
    int y = (int)floor((mousePosition.y - BOARD_Y) / SQUARE_SIZE);
    Move_t move = coordsToMove(x, y);
    return isMoveInBounds(move) ? move : MOVE_NONE;
}

/**
 * @brief Checks if mouse is over the "Play as Black" restart button
 * @return True if mouse is within button bounds
 */
bool isMousePointerOverPlayBlackButton() {
    return isMousePointerOverButton({ INFO_PLAYBLACK_BUTTON_X, INFO_PLAYBLACK_BUTTON_Y });
}

/**
 * @brief Checks if mouse is over the "Play as White" restart button
 * @return True if mouse is within button