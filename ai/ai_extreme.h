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

#include "../transposition_table.h"
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

  public:
    AIExtreme();
    virtual ~AIExtreme();

    virtual Move_t getBestMove(GameModel& model) override;

    virtual const char* getName() const override {
        return "Extreme AI (Advanced Search + TT)";
    }

    virtual void getSearchStats(int& nodesSearched, int& maxDepth) const override;
};


// ============================================================================
// AI Configuration Constants
// ============================================================================

// Search parameters
#define MAX_SEARCH_DEPTH 12   // Maximum search depth
#define TIME_LIMIT_MS 15000   // 15 seconds per move
#define ENDGAME_DEPTH 16      // Depth for endgame (when few discs remain)
#define ENDGAME_THRESHOLD 12  // Switch to deeper search when <= 12 empty

// Evaluation weights (can be tuned)
#define WEIGHT_MOBILITY 10   // Mobility importance
#define WEIGHT_CORNER 100    // Corner importance
#define WEIGHT_X_SQUARE -50  // X-square penalty (dangerous!)
#define WEIGHT_C_SQUARE -20  // C-square penalty
#define WEIGHT_EDGE 5        // Edge bonus
#define WEIGHT_STABILITY 15  // Stable disc bonus
#define WEIGHT_FRONTIER -5   // Frontier disc penalty

// Score constants
#define INFINITY_SCORE 1000000  // Represents "infinity" for alpha-beta
#define WIN_SCORE 100000        // Winning position score
#define LOSE_SCORE -100000      // Losing position score

// ============================================================================
// Evaluator Class - Evaluates board positions
// ============================================================================

class Evaluator {
  private:
    // Static piece-square table (positional values)
    static const int pieceSquareTable[64];

  public:
    /**
     * @brief Main evaluation function - combines all heuristics
     */
    int evaluate(const Board_t& board, PlayerColor_t player);

    /**
     * @brief Evaluates mobility (number of legal moves)
     */
    int evaluateMobility(const Board_t& board, PlayerColor_t player);

    /**
     * @brief Evaluates corner control
     */
    int evaluateCorners(const Board_t& board, PlayerColor_t player);

    /**
     * @brief Evaluates using positional weights (piece-square table)
     */
    int evaluatePositional(const Board_t& board, PlayerColor_t player);

    /**
     * @brief Evaluates stability (discs that cannot be flipped)
     */
    int evaluateStability(const Board_t& board, PlayerColor_t player);

    /**
     * @brief Evaluates frontier discs (pieces adjacent to empty squares)
     */
    int evaluateFrontier(const Board_t& board, PlayerColor_t player);

    /**
     * @brief Evaluates disc parity (piece count difference)
     */
    int evaluateDiscParity(const Board_t& board, PlayerColor_t player);
};

// ============================================================================
// SearchEngine Class - Implements search algorithms
// ============================================================================

class SearchEngine {
  private:
    Evaluator evaluator;
    TranspositionTable tt;  // Transposition table

    // Search statistics
    int nodesSearched;
    int cutoffs;
    int maxDepthReached;

    // Move ordering
    Move_t pvMove;  // Principal variation move from previous iteration

    // Time management
    std::chrono::time_point<std::chrono::high_resolution_clock> searchStartTime;
    double timeLimit;  // In seconds

    /**
     * @brief Checks if time limit has been exceeded
     */
    bool isTimeUp();

  public:
    SearchEngine();

    /**
     * @brief Main search function - uses iterative deepening
     */
    Move_t search(Board_t& board, PlayerColor_t player, double timeLimitSeconds);

    /**
     * @brief Negamax search with alpha-beta pruning and TT
     */
    int negamax(
        Board_t& board, PlayerColor_t player, int depth, int alpha, int beta, uint64_t hash);

    /**
     * @brief Root-level search (finds best move, not just score)
     */
    Move_t rootSearch(Board_t& board, PlayerColor_t player, int depth, int alpha, int beta);

    /**
     * @brief Orders moves for better pruning (best moves first)
     */
    void orderMoves(MoveList& moves, const Board_t& board, PlayerColor_t player);

    /**
     * @brief Scores a move for ordering purposes
     */
    int scoreMoveForOrdering(Move_t move, const Board_t& board, PlayerColor_t player);

    /**
     * @brief Quiescence search - extends search in tactical positions
     */
    int quiescence(Board_t& board, PlayerColor_t player, int alpha, int beta);

    // Getters for statistics
    int getNodesSearched() const;
    int getCutoffs() const;
    int getMaxDepthReached() const;
};

// ============================================================================
// Main AI Interface (called by controller)
// ============================================================================

/**
 * @brief Returns the best move for the current position
 *
 * @param model The game model
 * @return The best move (Move_t 0-63), or MOVE_NONE if no valid moves
 */
Move_t getBestMove(GameModel& model);

#endif
