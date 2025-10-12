/**
 * @brief In-game settings overlay
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef SETTINGS_OVERLAY_H
#define SETTINGS_OVERLAY_H

#include <string>

 // Settings overlay rendering
void drawSettingsButton();
void drawAISettingsOverlay(const std::string& aiDifficulty, int nodeLimit);
void drawSettingsOverlay();

// Settings button handlers
bool isMousePointerOverSettingsButton();
bool isMousePointerOverAIDifficultyButton();
bool isMousePointerOverAINodeLimitSlider();
bool isMousePointerOverAIMainMenuButton();
bool isMousePointerOverCloseAISettingsButton();
bool isMousePointerOverConfirmAISettingsButton();
bool isMousePointerOverCloseSettingsButton();
bool isMousePointerOverMainMenuButton();

#endif // SETTINGS_OVERLAY_H
