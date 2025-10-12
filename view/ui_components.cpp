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
void drawButton(Vector2 position, const std::string& label, Color backgroundColor) {
    DrawRectangle(position.x - INFO_BUTTON_WIDTH / 2,
        position.y - INFO_BUTTON_HEIGHT / 2,
        INFO_BUTTON_WIDTH,
        INFO_BUTTON_HEIGHT,
        backgroundColor);

    drawCenteredText({ position.x, position.y }, SUBTITLE_FONT_SIZE, label.c_str());
}

/**
 * @brief Draws a rectangular button with centered text and custom text color
 * @param position Center position of the button
 * @param label Text to display on the button
 * @param backgroundColor Background color of the button
 * @param textColor Text color
 */
void drawColoredButton(Vector2 position, const std::string& label, Color backgroundColor, Color textColor) {
    DrawRectangle(position.x - INFO_BUTTON_WIDTH / 2,
        position.y - INFO_BUTTON_HEIGHT / 2,
        INFO_BUTTON_WIDTH,
        INFO_BUTTON_HEIGHT,
        backgroundColor);

    drawCenteredColoredText({ position.x, position.y }, SUBTITLE_FONT_SIZE, label, textColor);
}

/**
 * @brief Draws a rectangular button with centered text, custom text color, and custom font size
 * @param position Center position of the button
 * @param label Text to display on the button
 * @param backgroundColor Background color of the button
 * @param textColor Text color
 * @param fontSize Font size for the text
 */
void drawButtonWithFont(Vector2 position, const std::string& label, Color backgroundColor, Color textColor, int fontSize) {
    // Draw the button background rectangle centered at position
    DrawRectangle(position.x - INFO_BUTTON_WIDTH / 2,
        position.y - INFO_BUTTON_HEIGHT / 2,
        INFO_BUTTON_WIDTH,
        INFO_BUTTON_HEIGHT,
        backgroundColor);

    // Draw the label centered inside the button with custom color and font size
    drawCenteredColoredText({ position.x, position.y }, fontSize, label, textColor);
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

/**
 * @brief Draws a horizontal slider with label and current value
 * @param position Center position of the slider
 * @param width Width of the slider track
 * @param minValue Minimum value of the slider
 * @param maxValue Maximum value of the slider
 * @param currentValue Current value to display
 * @param label Label text to display above slider
 */
void drawSlider(Vector2 position, float width, int minValue, int maxValue, int currentValue, const std::string& label) {
    // Draw label
    std::string labelText = label + ": " + std::to_string(currentValue);
    drawCenteredColoredText({ position.x, position.y - 25 }, NORMAL_FONT_SIZE, labelText, DARKBROWN);

    // Calculate handle position
    float normalizedValue = (float)(currentValue - minValue) / (float)(maxValue - minValue);
    float handleX = position.x - width / 2 + normalizedValue * width;

    // Draw slider track
    DrawRectangle((int)(position.x - width / 2), (int)(position.y - 3),
        (int)width, 6, GRAY);

    // Draw filled portion
    DrawRectangle((int)(position.x - width / 2), (int)(position.y - 3),
        (int)(handleX - (position.x - width / 2)), 6, DARKBROWN);

    // Draw handle
    DrawCircle((int)handleX, (int)position.y, 12, DARKBROWN);
    DrawCircle((int)handleX, (int)position.y, 10, BEIGE);

    // Draw min/max labels
    drawCenteredColoredText({ position.x - width / 2, position.y + 20 },
        18, std::to_string(minValue), GRAY);
    drawCenteredColoredText({ position.x + width / 2, position.y + 20 },
        18, std::to_string(maxValue), GRAY);
}

/**
 * @brief Checks if mouse pointer is over slider area
 * @param position Center position of the slider
 * @param width Width of the slider
 * @return true if mouse is over slider, false otherwise
 */
bool isMousePointerOverSlider(Vector2 position, float width) {
    Vector2 mousePos = GetMousePosition();

    return (mousePos.x >= position.x - width / 2 - 15 &&
        mousePos.x <= position.x + width / 2 + 15 &&
        mousePos.y >= position.y - 15 &&
        mousePos.y <= position.y + 15);
}

/**
 * @brief Calculates slider value from mouse position
 * @param mousePos Current mouse position
 * @param sliderPos Center position of the slider
 * @param width Width of the slider
 * @param minValue Minimum value
 * @param maxValue Maximum value
 * @return Calculated value based on mouse position
 */
int getSliderValue(Vector2 mousePos, Vector2 sliderPos, float width, int minValue, int maxValue) {
    float relativeX = mousePos.x - (sliderPos.x - width / 2);
    float normalizedValue = relativeX / width;

    // Clamp between 0 and 1
    if (normalizedValue < 0) normalizedValue = 0;
    if (normalizedValue > 1) normalizedValue = 1;

    int value = minValue + (int)(normalizedValue * (maxValue - minValue));

    // Round to nearest 100
    value = ((value + 50) / 100) * 100;

    return value;
}