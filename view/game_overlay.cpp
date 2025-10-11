/**
 * @brief Game overlay rendering implementation
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "game_overlay.h"
#include "view_constants.h"
#include "ui_components.h"
#include "raylib.h"

 /**
  * @brief Draws pass turn message overlay
  * @param model Game model containing pass message state
  */
void drawPassMessage(const GameModel& model) {
    if (!model.showPassMessage) {
        return;
    }

    std::string passMessage = (model.passedPlayer == PLAYER_BLACK) ? "BLACK PASSES TURN" : "WHITE PASSES TURN";
    Color messageColor = (model.passedPlayer == PLAYER_BLACK) ? BLACK : WHITE;
    Color bgColor = (model.passedPlayer == PLAYER_BLACK) ? WHITE : BLACK;

    // Semi-transparent background
    DrawRectangle(BOARD_X, BOARD_Y + BOARD_CONTENT_SIZE / 2 - 50,
        BOARD_CONTENT_SIZE, 100,
        Fade(bgColor, 0.85f));

    // Message text
    int messageWidth = MeasureText(passMessage.c_str(), SUBTITLE_FONT_SIZE);
    DrawText(passMessage.c_str(),
        BOARD_X + BOARD_CONTENT_SIZE / 2 - messageWidth / 2,
        BOARD_Y + BOARD_CONTENT_SIZE / 2 - SUBTITLE_FONT_SIZE / 2,
        SUBTITLE_FONT_SIZE,
        messageColor);
}

/**
 * @brief Draws game over screen with winner information
 * @param model Game model containing final scores
 */
void drawGameOverScreen(const GameModel& model) {
    if (!model.gameOver) {
        return;
    }

    // Draw new game buttons
    drawButton({ INFO_PLAYBLACK_BUTTON_X, INFO_PLAYBLACK_BUTTON_Y }, "Play black", BLACK);
    drawButton({ INFO_PLAYWHITE_BUTTON_X, INFO_PLAYWHITE_BUTTON_Y }, "Play white", WHITE);

    // Calculate scores and determine winner
    int blackScore = getScore(model, PLAYER_BLACK);
    int whiteScore = getScore(model, PLAYER_WHITE);

    if (!blackScore && !whiteScore) {
        return;
    }

    std::string titleText;
    std::string diffText;
    Color titleColor;
    Color bgColor;

    if (blackScore > whiteScore) {
        titleText = "BLACK WINS";
        titleColor = BLACK;
        bgColor = Fade(WHITE, 0.75f); // contrasting background
        diffText = "Difference: " + std::to_string(blackScore - whiteScore);
    }
    else if (whiteScore > blackScore) {
        titleText = "WHITE WINS";
        titleColor = WHITE;
        bgColor = Fade(BLACK, 0.75f); // contrasting background
        diffText = "Difference: " + std::to_string(whiteScore - blackScore);
    }
    else {
        titleText = "DRAW";
        titleColor = BEIGE;
        bgColor = Fade(DARKGRAY, 0.75f); // soft purple mixed background
        diffText = "Equal score: " + std::to_string(blackScore) + " - " + std::to_string(whiteScore);
    }

    // Centered semi-transparent overlay
    int overlayW = BOARD_CONTENT_SIZE;
    int overlayH = SQUARE_SIZE * 2.5;
    int overlayX = BOARD_X;
    int overlayY = BOARD_Y + BOARD_CONTENT_SIZE / 2 - overlayH / 2;

    // Overlay background (slightly opaque, uses bgColor if good contrast)
    DrawRectangle(overlayX, overlayY, overlayW, overlayH, bgColor);
    // Subtle border with winner's color
    DrawRectangleLines(overlayX, overlayY, overlayW, overlayH, titleColor);

    // Centered messages within overlay
    Vector2 overlayCenter = { (float)(overlayX + overlayW / 2), (float)(overlayY + overlayH / 2) };

    // Large title (uses winner's color; if winner is WHITE, background is black so WHITE will be visible)
    drawCenteredColoredText({ overlayCenter.x, overlayCenter.y - 20 }, TITLE_FONT_SIZE, titleText, titleColor);

    // Smaller text with the difference
    drawCenteredColoredText({ overlayCenter.x, overlayCenter.y + 28 }, SUBTITLE_FONT_SIZE, diffText, titleColor);
}