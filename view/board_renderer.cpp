/**
 * @brief Board and piece rendering functionality implementation
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "board_renderer.h"
#include "view_constants.h"
#include "raylib.h"

 /**
  * @brief Renders the game board with squares and border
  */
void drawBoard() {
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
}

/**
 * @brief Renders all game pieces on the board
 * @param model Game model containing piece positions
 */
void drawBoardPieces(const GameModel& model) {
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
}

/**
 * @brief Highlights valid moves with flip counts
 * @param model Game model containing valid moves and board state
 */
void drawValidMoves(const GameModel& model) {
    if (model.gameOver || model.showPassMessage) {
        return;
    }

    MoveList validMoves;
    getValidMoves(model, validMoves);

    // Get current player's bitboards
    uint64_t player = getPlayerBitboard(model.board, getCurrentPlayer(model));
    uint64_t opponent = getOpponentBitboard(model.board, getCurrentPlayer(model));
    Color highlightColor = (getCurrentPlayer(model) == PLAYER_BLACK) ? BLACK : WHITE;

    for (Move_t move : validMoves) {
        int x = getMoveX(move);
        int y = getMoveY(move);

        // Calculate circle center
        int centerX = (int)(BOARD_X + (float)x * SQUARE_SIZE) + PIECE_CENTER;
        int centerY = (int)(BOARD_Y + (float)y * SQUARE_SIZE) + PIECE_CENTER;

        // Draw highlight circle
        DrawCircle(centerX, centerY, PIECE_RADIUS, highlightColor);
        DrawCircle(centerX, centerY, PIECE_RADIUS * 0.8, DARKGREEN);

        // Calculate number of pieces that would be flipped
        uint64_t flips = calculateFlips(player, opponent, move);
        int flipCount = countBits(flips);

        const char* text = TextFormat("%d", flipCount);
        int fontSize = 30;
        int textWidth = MeasureText(text, fontSize);
        // Center text in X and Y
        DrawText(text, centerX - textWidth / 2, centerY - fontSize / 2, fontSize, highlightColor);
    }
}