/**
 * @brief UI rendering components and utilities
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include "raylib.h"
#include <string>

// Text rendering
void drawCenteredText(Vector2 position, int fontSize, std::string text);
void drawCenteredColoredText(Vector2 position, int fontSize, const std::string& text, Color color);

// Score and timer display
void drawScore(std::string label, Vector2 position, int score);
void drawTimer(Vector2 position, double time);

// Button components
void drawButton(Vector2 position, const std::string& label, Color backgroundColor);
void drawColoredButton(Vector2 position, const std::string& label, Color backgroundColor, Color textColor);
void drawButtonWithFont(Vector2 position, const std::string& label, Color backgroundColor, Color textColor, int fontSize);
bool isMousePointerOverButton(Vector2 position);

// Slider component
void drawSlider(Vector2 position, float width, int minValue, int maxValue, int currentValue, const std::string& label);
bool isMousePointerOverSlider(Vector2 position, float width);
int getSliderValue(Vector2 mousePos, Vector2 sliderPos, float width, int minValue, int maxValue);

#endif // UI_COMPONENTS_H