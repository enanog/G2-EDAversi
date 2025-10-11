/**
 * @brief Hard difficulty AI - Minimax with Alpha-Beta pruning
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef AI_HARD_H
#define AI_HARD_H

#include "ai_interface.h"
#include <vector>

 /**
  * @brief Hard AI using Minimax with Alpha-Beta pruning
  * Features enhanced evaluation function with positional weights and mobility
  */
class AIHard : public AIInterface {
private:
    static const int MAX_DEPTH = 8;
    static const int MAX_NODES = 500000;

    // Positional weight table emphasizing corners and edges
    static const int POSITION_WEIGHTS[64];

    mutable int nodesExplored;

    /**
     * @brief Creates a copy of the game model for simulation
     */
    inline GameModel copyModel(const GameModel& model) const;

    /**
     * @brief Advanced board evaluation with multiple heuristics
     * Combines positional scoring, mobility, corner control, and piece count
     */
    int evaluateBoard(const GameModel& model, PlayerColor_t maximizingPlayer) const;

    /**
     * @brief Minimax search with Alpha-Beta pruning
     * Explores game tree to specified depth with move ordering
     */
    int alphaBeta(GameModel& model, int depth, int alpha, int beta,
        bool isMaximizing, PlayerColor_t maximizingPlayer) const;

public:
    AIHard();
    virtual ~AIHard() = default;

    virtual Move_t getBestMove(GameModel& model) override;
    virtual const char* getName() const override {
        return "Hard AI (Alpha-Beta)";
    }

    virtual void getSearchStats(int& nodesSearched, int& maxDepth) const override {
        nodesSearched = this->nodesExplored;
        maxDepth = MAX_DEPTH;
    }
};

#endif // AI_HARD_H