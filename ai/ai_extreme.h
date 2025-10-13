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

#include <chrono>
#include <memory>
#include <vector>

#include "opening_book.h"
#include "transposition_table.h"
#include "ai_interface.h"

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
    std::unique_ptr<OpeningBook> book;
    int moveCount;  // Track move number for book depth

public:
    AIExtreme();
    virtual ~AIExtreme();

    /**
     * @brief Loads opening book from file or directory
     * @param path Path to .wtb file or directory containing .wtb files
     * @return Number of games loaded
     */
    int loadOpeningBook(const std::string& path);

    virtual Move_t getBestMove(GameModel& model) override;

    virtual const char* getName() const override {
        return "Extreme AI (Advanced Search + TT)";
    }

    virtual void getSearchStats(int& nodesSearched, int& maxDepth) const override;

    // Implementación de gestión de límite de nodos
    virtual void setNodeLimit(int limit) override;
    virtual int getNodeLimit() const override;
};

#endif // AI_EXTREME_H