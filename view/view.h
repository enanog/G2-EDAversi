/**
 * @brief Main view interface for Reversi game - Centralized header
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef VIEW_H
#define VIEW_H

#include "../model.h"
#include <string>
#include <raylib.h>

 // ============================================================================
 // CORE VIEW FUNCTIONS (view.cpp)
 // ============================================================================

void initView();
void freeView();
void drawView(GameModel& model, bool showSettings = false,
    const std::string& aiDifficulty = "Normal", int nodeLimit = 500000, bool aiActivate = true);
Move_t getMoveOnMousePointer();

// ============================================================================
// MENU SYSTEM (menu_system.cpp)
// ============================================================================

void drawMainMenu();
void drawAIDifficultyMenu();

// Menu button handlers
bool isMousePointerOverMenu1v1Button();
bool isMousePointerOverMenu1vAIButton();
bool isMousePointerOverMenuSettingsButton();

// Game over button handlers
bool isMousePointerOverPlayBlackButton();
bool isMousePointerOverPlayWhiteButton();

// AI difficulty button handlers
bool isMousePointerOverAIEasyButton();
bool isMousePointerOverAINormalButton();
bool isMousePointerOverAIHardButton();
bool isMousePointerOverAIExtremeButton();
bool isMousePointerOverBackToMenuButton();
bool isMousePointerOverContinueToMenuButton();

// ============================================================================
// SETTINGS OVERLAY (settings_overlay.cpp)
// ============================================================================

bool isMousePointerOverSettingsButton();
bool isMousePointerOverAIDifficultyButton();
bool isMousePointerOverAINodeLimitSlider();
bool isMousePointerOverAIMainMenuButton();
bool isMousePointerOverCloseAISettingsButton();
bool isMousePointerOverConfirmAISettingsButton();
bool isMousePointerOverCloseSettingsButton();
bool isMousePointerOverMainMenuButton();

// ============================================================================
// UI COMPONENTS (ui_components.cpp)
// ============================================================================

/**
 * @brief Calculates slider value from mouse position
 * @param mousePos Current mouse position
 * @param sliderPos Center position of the slider
 * @param width Width of the slider
 * @param minValue Minimum value
 * @param maxValue Maximum value
 * @return Calculated value based on mouse position
 */
int getSliderValue(Vector2 mousePos, Vector2 sliderPos, float width, int minValue, int maxValue);

#endif // VIEW_H