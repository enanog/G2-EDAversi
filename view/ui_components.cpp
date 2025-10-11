/**
 * @brief UI rendering components and utilities implementation
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "ui_components.h"
#include "view_constants.h"
#include <string>

/**
 * @brief Draws text centered around specified position
 * @param position Center point for text
 * @param fontSize Font size for text rendering
 * @param text String to display
 */
void drawCenteredText(Vector2 position, int fontSize, std::string text) {
    DrawText(text.c_str(),
        (int)position.x - MeasureText(text.c_str(), fontSize) / 2,
        (int)position.y - fontSize / 2,
        fontSize,
        BROWN);
}

/**
 * @brief Draws text centered with custom color
 * @param position Center point for text
 * @param fontSize Font size for text rendering
 * @param text String to display
 * @param color Text color
 */
void drawCenteredColoredText(Vector2 position, int fontSize, const std::string& text, Color color) {
    int textWidth = MeasureText(text.c_str(), fontSize);
    DrawText(text.c_str(),
        (int)position.x - textWidth / 2,
        (int)position.y - fontSize / 2,
        fontSize,
        color);
}

/**
 * @brief Draws player score with label
 * @param label Score label (e.g., "Black score: ")
 * @param position Position to draw the score
 * @param score Numerical score value
 */
void drawScore(std::string label, Vector2 position, int score) {
    std::string displayText = label + std::to_string(score);
    drawCenteredText(position, SUBTITLE_FONT_SIZE, displayText);
}

/**
 * @brief Draws formatted timer (MM:SS)
 * @param position Position to draw the timer
 * @param time Time in seconds to format and display
 */
void drawTimer(Vector2 position, double time) {
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
 * @param position Center position of the button
 * @param label Text to display on the button
 * @param backgroundColor Background color of the button
 */
void drawButton(Vector2 position, std::string label, Color backgroundColor) {
    DrawRectangle(position.x - INFO_BUTTON_WIDTH / 2,
        position.y - INFO_BUTTON_HEIGHT / 2,
        INFO_BUTTON_WIDTH,
        INFO_BUTTON_HEIGHT,
        backgroundColor);

    drawCenteredText({ position.x, position.y }, SUBTITLE_FONT_SIZE, label.c_str());
}

/**
 * @brief Checks if mouse pointer is within button bounds
 * @param position Center position of the button to check
 * @return true if mouse is over button, false otherwise
 */
bool isMousePointerOverButton(Vector2 position) {
    Vector2 mousePosition = GetMousePosition();

    return ((mousePosition.x >= (position.x - INFO_BUTTON_WIDTH / 2)) &&
        (mousePosition.x < (position.x + INFO_BUTTON_WIDTH / 2)) &&
        (mousePosition.y >= (position.y - INFO_BUTTON_HEIGHT / 2)) &&
        (mousePosition.y < (position.y + INFO_BUTTON_HEIGHT / 2)));
}