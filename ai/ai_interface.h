/**
 * @brief Abstract interface for Reversi AI implementations
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef AI_INTERFACE_H
#define AI_INTERFACE_H

#include "../model.h"

 /**
  * @brief AI difficulty levels
  */
enum AIDifficulty {
    AI_EASY,      // Random valid moves
    AI_NORMAL,    // Basic Minimax
    AI_HARD,      // Minimax with Alpha-Beta pruning
    AI_EXTREME    // Negamax with Transposition Tables
};

/**
 * @brief Abstract base class for all AI implementations
 * Enables polymorphism and easy swapping of AI strategies
 */
class AIInterface {
protected:
    AIInterface() = default;

public:
    virtual ~AIInterface() = default;

    /**
     * @brief Calculates best move for current position
     * @param model Current game state
     * @return Best move (0-63) or MOVE_NONE if no moves available
     */
    virtual Move_t getBestMove(GameModel& model) = 0;

    /**
     * @brief Gets descriptive AI name for display
     */
    virtual const char* getName() const = 0;

    /**
     * @brief Retrieves search statistics if available
     * @param nodesSearched Number of nodes explored
     * @param maxDepth Maximum search depth reached
     */
    virtual void getSearchStats(int& nodesSearched, int& maxDepth) const {
        nodesSearched = 0;
        maxDepth = 0;
    }

    /**
     * @brief Resets internal AI state if needed
     */
    virtual void reset() {}
};

#endif // AI_INTERFACE_H