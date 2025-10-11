/**
 * @brief Menu system and button handlers implementation
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "menu_system.h"
#include "view_constants.h"
#include "ui_components.h"
#include "raylib.h"

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
 * Handles its own BeginDrawing/EndDrawing calls
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

// Button handler implementations

bool isMousePointerOverPlayBlackButton() {
    return isMousePointerOverButton({ INFO_PLAYBLACK_BUTTON_X, INFO_PLAYBLACK_BUTTON_Y });
}

bool isMousePointerOverPlayWhiteButton() {
    return isMousePointerOverButton({ INFO_PLAYWHITE_BUTTON_X, INFO_PLAYWHITE_BUTTON_Y });
}

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