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
void drawAIDifficultyButtons();

// Button handlers
bool isMousePointerOverPlayBlackButton();
bool isMousePointerOverPlayWhiteButton();
bool isMousePointerOverAIEasyButton();
bool isMousePointerOverAINormalButton();
bool isMousePointerOverAIHardButton();
bool isMousePointerOverAIExtremeButton();

#endif // MENU_SYSTEM_H