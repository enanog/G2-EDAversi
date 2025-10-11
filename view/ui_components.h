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
void drawButton(Vector2 position, std::string label, Color backgroundColor);
bool isMousePointerOverButton(Vector2 position);

#endif // UI_COMPONENTS_H