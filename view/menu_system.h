/**
 * @brief Menu system and button handlers
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

 // Menu rendering
void drawMainMenu();
void drawAIDifficultyMenu();

// Main menu button handlers
bool isMousePointerOverMenu1v1Button();
bool isMousePointerOverMenu1vAIButton();
bool isMousePointerOverMenuSettingsButton();

// Game over screen button handlers
bool isMousePointerOverPlayBlackButton();
bool isMousePointerOverPlayWhiteButton();

// AI difficulty selection handlers
bool isMousePointerOverAIEasyButton();
bool isMousePointerOverAINormalButton();
bool isMousePointerOverAIHardButton();
bool isMousePointerOverAIExtremeButton();
bool isMousePointerOverBackToMenuButton();
bool isMousePointerOverContinueToMenuButton();

#endif // MENU_SYSTEM_H