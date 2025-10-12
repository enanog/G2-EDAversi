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
  * @brief Draws main menu with game mode selection
  * Handles its own BeginDrawing/EndDrawing calls
  */
void drawMainMenu() {
    BeginDrawing();
    ClearBackground(BEIGE);

    // Title
    drawCenteredText({ (float)WINDOW_WIDTH / 2, (float)WINDOW_HEIGHT / 6 },
        TITLE_FONT_SIZE, GAME_NAME);

    drawCenteredText({ (float)WINDOW_WIDTH / 2, (float)WINDOW_HEIGHT / 6 + TITLE_FONT_SIZE * 0.8f },
        SUBTITLE_FONT_SIZE, "Select Game Mode");

    // Main menu options
    drawColoredButton({ MENU_BUTTON_X, MENU_1V1_BUTTON_Y }, "1 vs 1", DARKBROWN, WHITE);
    drawColoredButton({ MENU_BUTTON_X, MENU_1VAI_BUTTON_Y }, "1 vs AI", DARKBROWN, WHITE);
    drawColoredButton({ MENU_BUTTON_X, MENU_SETTINGS_BUTTON_Y }, "AI Settings", DARKGRAY, WHITE);

    EndDrawing();
}

/**
 * @brief Draws AI difficulty selection screen
 */
void drawAIDifficultyMenu() {
    BeginDrawing();
    ClearBackground(BEIGE);

    // Title
    drawCenteredText({ (float)WINDOW_WIDTH / 2, (float)WINDOW_HEIGHT / 6 },
        TITLE_FONT_SIZE, "AI Settings");

    drawCenteredText({ (float)WINDOW_WIDTH / 2, (float)WINDOW_HEIGHT / 6 + TITLE_FONT_SIZE * 0.8f },
        SUBTITLE_FONT_SIZE, "Configure AI Difficulty");

    // Difficulty buttons using macros
    drawColoredButton({ (float)WINDOW_WIDTH / 2, DIFFICULTY_BUTTON_Y(0) }, "Easy AI", DARKBROWN, WHITE);
    drawColoredButton({ (float)WINDOW_WIDTH / 2, DIFFICULTY_BUTTON_Y(1) }, "Normal AI", DARKBROWN, WHITE);
    drawColoredButton({ (float)WINDOW_WIDTH / 2, DIFFICULTY_BUTTON_Y(2) }, "Hard AI", DARKBROWN, WHITE);
    drawColoredButton({ (float)WINDOW_WIDTH / 2, DIFFICULTY_BUTTON_Y(3) }, "Extreme AI", DARKBROWN, WHITE);

    // Action buttons
    drawColoredButton({ DIFFICULTY_BACK_BUTTON_X, DIFFICULTY_BACK_BUTTON_Y }, "Back", RED, WHITE);
    drawColoredButton({ DIFFICULTY_CONFIRM_BUTTON_X, DIFFICULTY_CONFIRM_BUTTON_Y }, "Confirm", GREEN, WHITE);

    EndDrawing();
}

// Main menu button handlers
bool isMousePointerOverMenu1v1Button() {
    return isMousePointerOverButton({ MENU_BUTTON_X, MENU_1V1_BUTTON_Y });
}

bool isMousePointerOverMenu1vAIButton() {
    return isMousePointerOverButton({ MENU_BUTTON_X, MENU_1VAI_BUTTON_Y });
}

bool isMousePointerOverMenuSettingsButton() {
    return isMousePointerOverButton({ MENU_BUTTON_X, MENU_SETTINGS_BUTTON_Y });
}

// Game over screen button handlers
bool isMousePointerOverPlayBlackButton() {
    return isMousePointerOverButton({ INFO_PLAYBLACK_BUTTON_X, INFO_PLAYBLACK_BUTTON_Y });
}

bool isMousePointerOverPlayWhiteButton() {
    return isMousePointerOverButton({ INFO_PLAYWHITE_BUTTON_X, INFO_PLAYWHITE_BUTTON_Y });
}

// AI difficulty selection handlers
bool isMousePointerOverAIEasyButton() {
    return isMousePointerOverButton({ (float)WINDOW_WIDTH / 2, DIFFICULTY_BUTTON_Y(0) });
}

bool isMousePointerOverAINormalButton() {
    return isMousePointerOverButton({ (float)WINDOW_WIDTH / 2, DIFFICULTY_BUTTON_Y(1) });
}

bool isMousePointerOverAIHardButton() {
    return isMousePointerOverButton({ (float)WINDOW_WIDTH / 2, DIFFICULTY_BUTTON_Y(2) });
}

bool isMousePointerOverAIExtremeButton() {
    return isMousePointerOverButton({ (float)WINDOW_WIDTH / 2, DIFFICULTY_BUTTON_Y(3) });
}

bool isMousePointerOverBackToMenuButton() {
    return isMousePointerOverButton({ DIFFICULTY_BACK_BUTTON_X, DIFFICULTY_BACK_BUTTON_Y });
}

bool isMousePointerOverContinueToMenuButton() {
    return isMousePointerOverButton({ DIFFICULTY_CONFIRM_BUTTON_X, DIFFICULTY_CONFIRM_BUTTON_Y });
}