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
void drawSettingsOverlay(const std::string& aiDifficulty, int nodeLimit);

// Settings button handlers
bool isMousePointerOverSettingsButton();
bool isMousePointerOverDifficultyButton();
bool isMousePointerOverNodeLimitSlider();
bool isMousePointerOverMainMenuButton();
bool isMousePointerOverCloseSettingsButton();
bool isMousePointerOverConfirmSettingsButton();

#endif // SETTINGS_OVERLAY_H
