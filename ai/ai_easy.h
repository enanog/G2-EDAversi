/**
 * @brief Easy difficulty AI - Random moves
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef AI_EASY_H
#define AI_EASY_H

#include "ai_interface.h"
#include <random>

 /**
  * @brief Easy AI - Selects random valid moves
  * Suitable for beginners and testing baseline performance
  */
class AIEasy : public AIInterface {
private:
    /**
     * @brief Mersenne Twister random number generator
     * Better distribution and performance than rand()
     */
    mutable std::mt19937 rng;

public:
    AIEasy();
    virtual ~AIEasy() = default;

    virtual Move_t getBestMove(GameModel& model) override;

    virtual const char* getName() const override {
        return "Easy AI (Random)";
    }
};

#endif // AI_EASY_H