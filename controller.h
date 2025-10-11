/**
 * @brief Implements the Reversi game controller with AI threading
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "model.h"
#include "ai/ai_interface.h"

 /**
  * @brief Updates the game view and handles input
  * @param model The game model to update
  * @return false if view should close, true otherwise
  */
bool updateView(GameModel& model);

// AI management functions
/**
 * @brief Initializes AI system with specified difficulty
 * @param difficulty Desired AI difficulty level
 * Must be called ONCE before game loop starts
 */
void initializeAI(AIDifficulty difficulty);

/**
 * @brief Changes AI difficulty during runtime
 * @param difficulty New difficulty level
 * Can only be changed when AI is not thinking
 */
void changeAIDifficulty(AIDifficulty difficulty);

/**
 * @brief Gets current AI's descriptive name
 * @return AI name string
 */
const char* getCurrentAIName();

#endif // CONTROLLER_H