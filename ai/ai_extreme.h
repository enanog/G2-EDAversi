/**
 * @brief Extreme difficulty AI - Advanced search with Transposition Tables
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef AI_EXTREME_H
#define AI_EXTREME_H

#include "ai_interface.h"
#include "../transposition_table.h"
#include <memory>
#include <chrono>
#include <vector>

 /**
  * @brief Extreme difficulty AI using advanced search algorithms
  * Implements iterative deepening with transposition tables and sophisticated evaluation
  */
class AIExtreme : public AIInterface {
private:
    // Forward declarations for internal components
    class Evaluator;
    class SearchEngine;

    std::unique_ptr<SearchEngine> engine;

public:
    AIExtreme();
    virtual ~AIExtreme();

    virtual Move_t getBestMove(GameModel& model) override;

    virtual const char* getName() const override {
        return "Extreme AI (Advanced Search)";
    }

    virtual void getSearchStats(int& nodesSearched, int& maxDepth) const override {
        // Statistics would be implemented in SearchEngine
        nodesSearched = 0;
        maxDepth = 0;
    }
};

#endif // AI_EXTREME_H