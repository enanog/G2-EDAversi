/**
 * @brief Normal difficulty AI - Basic Minimax
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef AI_NORMAL_H
#define AI_NORMAL_H

#include "ai_interface.h"

 /**
  * @brief Normal AI - Basic Minimax without pruning
  * Explores all nodes to fixed depth with simple evaluation
  */
class AINormal : public AIInterface {
private:
    static const int MAX_DEPTH = 4;
    static const int DEFAULT_NODE_LIMIT = 500000;  // Conservative limit for basic minimax

    mutable int nodesExplored;
    int maxNodes;  // Dynamic node limit

    GameModel copyModel(const GameModel& model) const;
    int evaluateBoard(const GameModel& model, PlayerColor_t maximizingPlayer) const;
    int minimax(GameModel& model, int depth, bool isMaximizing, PlayerColor_t maximizingPlayer) const;

public:
    AINormal();
    virtual ~AINormal() = default;

    virtual Move_t getBestMove(GameModel& model) override;
    virtual const char* getName() const override { return "Normal AI (Basic Minimax)"; }

    virtual void getSearchStats(int& nodesSearched, int& maxDepth) const override {
        nodesSearched = this->nodesExplored;
        maxDepth = MAX_DEPTH;
    }

    virtual void setNodeLimit(int limit) override {
        maxNodes = (limit > 0) ? limit : DEFAULT_NODE_LIMIT;
    }

    virtual int getNodeLimit() const override {
        return maxNodes;
    }
};

#endif // AI_NORMAL_H