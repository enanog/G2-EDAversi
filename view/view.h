/**
 * @brief Main view interface for Reversi game
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

// Main view functions
void initView();
void freeView();
void drawView(GameModel& model, bool showSettings = false,
    const std::string& aiDifficulty = "Normal", int nodeLimit = 500000, bool aiActivate = true);
Move_t getMoveOnMousePointer();

// Main menu functions
void drawMainMenu();
bool isMousePointerOverMenu1v1Button();
bool isMousePointerOverMenu1vAIButton();
bool isMousePointerOverMenuSettingsButton();

// AI Settings menu functions
void drawAIDifficultyMenu();
bool isMousePointerOverAIEasyButton();
bool isMousePointerOverAINormalButton();
bool isMousePointerOverAIHardButton();
bool isMousePointerOverAIExtremeButton();
bool isMousePointerOverBackToMenuButton();
bool isMousePointerOverContinueToMenuButton();

// Game over screen functions
bool isMousePointerOverPlayBlackButton();
bool isMousePointerOverPlayWhiteButton();

// In-game settings overlay functions
void drawSettingsButton();
void drawSettingsOverlay(const std::string& aiDifficulty, int nodeLimit);
bool isMousePointerOverSettingsButton();
bool isMousePointerOverDifficultyButton();
bool isMousePointerOverNodeLimitMinusButton();
bool isMousePointerOverNodeLimitPlusButton();
bool isMousePointerOverMainMenuButton();
bool isMousePointerOverCloseSettingsButton();

#endif // VIEW_H