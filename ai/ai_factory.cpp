/**
 * @brief Factory implementation for AI creation
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "ai_factory.h"
#include "ai_easy.h"
#include "ai_normal.h"
#include "ai_hard.h"
#include "ai_extreme.h"
#include <iostream>

std::unique_ptr<AIInterface> AIFactory::createAI(AIDifficulty difficulty) {
    std::cout << "[AIFactory] Creating AI: " << getDifficultyName(difficulty) << std::endl;

    switch (difficulty) {
    case AI_EASY:
        return std::make_unique<AIEasy>();
    case AI_NORMAL:
        return std::make_unique<AINormal>();
    case AI_HARD:
        return std::make_unique<AIHard>();
    case AI_EXTREME:
        return std::make_unique<AIExtreme>();
    default:
        std::cerr << "[AIFactory] Warning: Unknown difficulty, defaulting to Easy" << std::endl;
        return std::make_unique<AIEasy>();
    }
}

const char* AIFactory::getDifficultyName(AIDifficulty difficulty) {
    switch (difficulty) {
    case AI_EASY:
        return "Easy";
    case AI_NORMAL:
        return "Normal";
    case AI_HARD:
        return "Hard";
    case AI_EXTREME:
        return "Extreme";
    default:
        return "Unknown";
    }
}

bool AIFactory::isImplemented(AIDifficulty difficulty) {
    switch (difficulty) {
    case AI_EASY:
    case AI_NORMAL:
    case AI_HARD:
    case AI_EXTREME:
        return true;
    default:
        return false;
    }
}