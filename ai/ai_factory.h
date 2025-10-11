/**
 * @brief Factory for creating AI instances
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef AI_FACTORY_H
#define AI_FACTORY_H

#include "ai_interface.h"
#include <memory>

 /**
  * @brief Factory class for AI instance creation
  * Implements Factory pattern for encapsulated AI creation
  */
class AIFactory {
public:
    /**
     * @brief Creates AI instance of specified difficulty
     * @param difficulty Desired AI level
     * @return unique_ptr to created AI instance
     */
    static std::unique_ptr<AIInterface> createAI(AIDifficulty difficulty);

    /**
     * @brief Gets descriptive name for difficulty level
     * @param difficulty Level to name
     * @return String representation
     */
    static const char* getDifficultyName(AIDifficulty difficulty);

    /**
     * @brief Checks if difficulty level is implemented
     * @param difficulty Level to check
     * @return true if implemented and available
     */
    static bool isImplemented(AIDifficulty difficulty);
};

#endif // AI_FACTORY_H