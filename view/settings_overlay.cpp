/**
 * @brief In-game settings overlay implementation
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "settings_overlay.h"
#include "view_constants.h"
#include "ui_components.h"
#include "raylib.h"
#include <string>
#include <cmath>

 /**
  * @brief Draws settings gear icon button (in-game)
  */
void drawSettingsButton() {
    Vector2 pos = { SETTINGS_BUTTON_X, SETTINGS_BUTTON_Y };

    // Draw button background
    DrawCircle((int)pos.x, (int)pos.y, SETTINGS_ICON_SIZE / 2, GRAY);

    // Draw simple gear icon (8 teeth)
    int outerRadius = SETTINGS_ICON_SIZE / 2 - 5;
    int innerRadius = SETTINGS_ICON_SIZE / 2 - 12;

    for (int i = 0; i < 8; i++) {
        float angle = i * 45.0f * DEG2RAD;
        float x1 = pos.x + outerRadius * cos(angle);
        float y1 = pos.y + outerRadius * sin(angle);
        DrawCircle((int)x1, (int)y1, 3, DARKGRAY);
    }

    // Center circle
    DrawCircle((int)pos.x, (int)pos.y, innerRadius, LIGHTGRAY);
}

/**
 * @brief Draws settings overlay panel with configuration options
 * @param aiDifficulty Current AI difficulty name
 * @param nodeLimit Current node limit for AI
 */
void drawAISettingsOverlay(const std::string& aiDifficulty, int nodeLimit) {
    // Semi-transparent background
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.7f));

    // Settings panel
    DrawRectangle(SETTINGS_OVERLAY_X, AI_SETTINGS_OVERLAY_Y,
        SETTINGS_OVERLAY_WIDTH, AI_SETTINGS_OVERLAY_HEIGHT,
        BEIGE);
    DrawRectangleLines(SETTINGS_OVERLAY_X, AI_SETTINGS_OVERLAY_Y,
        SETTINGS_OVERLAY_WIDTH, AI_SETTINGS_OVERLAY_HEIGHT,
        DARKBROWN);

    // Title
    drawCenteredText({ SETTINGS_OVERLAY_X + SETTINGS_OVERLAY_WIDTH / 2.0f,
                      AI_SETTINGS_OVERLAY_Y + SUBTITLE_FONT_SIZE },
        SUBTITLE_FONT_SIZE, "Settings");

    // AI Difficulty setting
    std::string diffText = "Difficulty: " + aiDifficulty;
    DrawRectangle(SETTINGS_ITEM_X - 10, AI_SETTINGS_DIFFICULTY_Y,
        SETTINGS_ITEM_WIDTH + 20, 50, LIGHTGRAY);
    drawCenteredText({ SETTINGS_ITEM_X + SETTINGS_ITEM_WIDTH / 2.0f,
                      AI_SETTINGS_DIFFICULTY_Y + 25},
        NORMAL_FONT_SIZE, diffText);

    // Node Limit slider
    Vector2 sliderPos = { SETTINGS_OVERLAY_X + SETTINGS_OVERLAY_WIDTH / 2.0f,
                          AI_SETTINGS_NODE_LIMIT_Y };
    drawSlider(sliderPos, SLIDER_WIDTH, NODE_LIMIT_MIN, NODE_LIMIT_MAX,
        nodeLimit, "Node Limit");

    // Action buttons
    drawButtonWithFont({ SETTINGS_ITEM_X + SETTINGS_ITEM_WIDTH / 2.0f,
                AI_SETTINGS_CONFIRM_Y },
        "Confirm", DARKGREEN, WHITE, NORMAL_FONT_SIZE);

    drawButtonWithFont({ SETTINGS_ITEM_X + SETTINGS_ITEM_WIDTH / 2.0f,
                AI_SETTINGS_CLOSE_Y },
        "Close Settings", MAROON, WHITE, NORMAL_FONT_SIZE);

    drawButtonWithFont({ SETTINGS_ITEM_X + SETTINGS_ITEM_WIDTH / 2.0f,
                AI_SETTINGS_MAIN_MENU_Y },
        "Return to Main Menu", BROWN, WHITE, NORMAL_FONT_SIZE);
}

void drawSettingsOverlay() {
    // Semi-transparent background
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.7f));

    // Settings panel
    DrawRectangle(SETTINGS_OVERLAY_X, SETTINGS_OVERLAY_Y,
        SETTINGS_OVERLAY_WIDTH, SETTINGS_OVERLAY_HEIGHT,
        BEIGE);
    DrawRectangleLines(SETTINGS_OVERLAY_X, SETTINGS_OVERLAY_Y,
        SETTINGS_OVERLAY_WIDTH, SETTINGS_OVERLAY_HEIGHT,
        DARKBROWN);

    // Title
    drawCenteredText({ SETTINGS_OVERLAY_X + SETTINGS_OVERLAY_WIDTH / 2.0f,
                      SETTINGS_OVERLAY_Y + SUBTITLE_FONT_SIZE },
        SUBTITLE_FONT_SIZE, "Settings");

    drawButtonWithFont({ SETTINGS_ITEM_X + SETTINGS_ITEM_WIDTH / 2.0f,
                SETTINGS_CLOSE_Y },
        "Close Settings", MAROON, WHITE, NORMAL_FONT_SIZE);

    drawButtonWithFont({ SETTINGS_ITEM_X + SETTINGS_ITEM_WIDTH / 2.0f,
                SETTINGS_MAIN_MENU_Y },
        "Return to Main Menu", BROWN, WHITE, NORMAL_FONT_SIZE);
}

// Button detection functions

bool isMousePointerOverSettingsButton() {
    Vector2 mousePos = GetMousePosition();
    Vector2 buttonPos = { SETTINGS_BUTTON_X, SETTINGS_BUTTON_Y };

    float distance = sqrt(pow(mousePos.x - buttonPos.x, 2) +
        pow(mousePos.y - buttonPos.y, 2));

    return distance <= SETTINGS_ICON_SIZE / 2;
}

bool isMousePointerOverAIDifficultyButton() {
    Vector2 mousePos = GetMousePosition();

    return (mousePos.x >= SETTINGS_ITEM_X - 10 &&
        mousePos.x <= SETTINGS_ITEM_X + SETTINGS_ITEM_WIDTH + 10 &&
        mousePos.y >= AI_SETTINGS_DIFFICULTY_Y - 25 &&
        mousePos.y <= AI_SETTINGS_DIFFICULTY_Y + 25);
}

bool isMousePointerOverAINodeLimitSlider() {
    Vector2 sliderPos = { SETTINGS_OVERLAY_X + SETTINGS_OVERLAY_WIDTH / 2.0f,
                          AI_SETTINGS_NODE_LIMIT_Y };
    return isMousePointerOverSlider(sliderPos, SLIDER_WIDTH);
}

bool isMousePointerOverAIMainMenuButton() {
    return isMousePointerOverButton({ SETTINGS_ITEM_X + SETTINGS_ITEM_WIDTH / 2.0f,
                                     AI_SETTINGS_MAIN_MENU_Y });
}

bool isMousePointerOverCloseAISettingsButton() {
    return isMousePointerOverButton({ SETTINGS_ITEM_X + SETTINGS_ITEM_WIDTH / 2.0f,
                                     AI_SETTINGS_CLOSE_Y });
}

bool isMousePointerOverConfirmAISettingsButton() {
    return isMousePointerOverButton({ SETTINGS_ITEM_X + SETTINGS_ITEM_WIDTH / 2.0f,
                                     AI_SETTINGS_CONFIRM_Y });
}

bool isMousePointerOverCloseSettingsButton() {
    return isMousePointerOverButton({ SETTINGS_ITEM_X + SETTINGS_ITEM_WIDTH / 2.0f,
                                     SETTINGS_CLOSE_Y });
}

bool isMousePointerOverMainMenuButton() {
    return isMousePointerOverButton({ SETTINGS_ITEM_X + SETTINGS_ITEM_WIDTH / 2.0f,
                                     SETTINGS_MAIN_MENU_Y });
}