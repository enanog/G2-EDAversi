/**
 * @brief Opening Book for Reversi AI
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef OPENING_BOOK_H
#define OPENING_BOOK_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../model.h"
#include "transposition_table.h"

// ============================================================================
// Opening Book Configuration
// ============================================================================

#define BOOK_MAX_DEPTH 12      // Store first 12 moves of each game
#define BOOK_MIN_GAME_COUNT 2  // Minimum games to consider move
#define BOOK_RANDOMNESS 0.15   // 15% chance to pick 2nd best move

// ============================================================================
// Book Move Entry
// ============================================================================

/**
 * @brief Statistics for a single move in a position
 */
struct BookMove {
    Move_t move;    // The move
    int gameCount;  // How many games used this move
    int winCount;   // How many of those games were won
    int drawCount;  // How many were draws
    int lossCount;  // How many were losses

    BookMove() : move(MOVE_NONE), gameCount(0), winCount(0), drawCount(0), lossCount(0) {
    }

    /**
     * @brief Calculates win rate for this move
     */
    double getWinRate() const {
        if (gameCount == 0)
            return 0.0;
        return (double)(winCount + drawCount * 0.5) / gameCount;
    }

    /**
     * @brief Score for move selection (higher is better)
     */
    double getScore() const {
        // Prioritize: win rate (80%) + game count popularity (20%)
        double winRate = getWinRate();
        double popularity = std::min(1.0, gameCount / 100.0);
        return winRate * 0.8 + popularity * 0.2;
    }
};

/**
 * @brief All moves available from a position
 */
struct BookPosition {
    std::vector<BookMove> moves;  // All moves tried in this position
    int totalGames;               // Total games that reached this position

    BookPosition() : totalGames(0) {
    }
};

// ============================================================================
// Opening Book Class
// ============================================================================

class OpeningBook {
  private:
    // Book storage: hash -> position data
    std::unordered_map<uint64_t, BookPosition> book;

    // Statistics
    int totalGamesLoaded;
    int totalPositions;
    int maxDepthStored;

    // Zobrist hashing (same as TT for consistency)
    TranspositionTable* tt;

    /**
     * @brief Converts WThor move encoding to our Move_t format
     *
     * WThor format: (row * 10 + col + 10)
     * Our format: (row * 8 + col)
     *
     * @param wthorMove WThor encoded move
     * @return Move_t (0-63), or MOVE_NONE if invalid
     */
    Move_t decodeWThorMove(uint8_t wthorMove) const;

    /**
     * @brief Parses a single .wtb file
     *
     * @param filename Path to .wtb file
     * @return Number of games loaded
     */
    int loadWTBFile(const std::string& filename);

    /**
     * @brief Adds a single game to the book
     *
     * @param moves List of moves in the game
     * @param blackScore Final score for black (0-64)
     */
    void addGame(const std::vector<Move_t>& moves, int blackScore);

  public:
    /**
     * @brief Constructor
     *
     * @param transpositionTable Shared TT for consistent hashing
     */
    OpeningBook(TranspositionTable* transpositionTable);

    /**
     * @brief Loads opening book from WThor database files
     *
     * @param directory Directory containing .wtb files
     * @return Number of games loaded
     */
    int load(const std::string& directory);

    /**
     * @brief Loads a single .wtb file
     *
     * @param filename Full path to .wtb file
     * @return Number of games loaded
     */
    int loadFile(const std::string& filename);

    /**
     * @brief Queries book for best move in position
     *
     * @param board Current board state
     * @param player Player to move
     * @param moveCount Current move number (for depth check)
     * @return Best book move, or MOVE_NONE if position not in book
     */
    Move_t probe(const Board_t& board, PlayerColor_t player, int moveCount);

    /**
     * @brief Checks if position is in book
     *
     * @param board Current board state
     * @param player Player to move
     * @return True if position found in book
     */
    bool contains(const Board_t& board, PlayerColor_t player) const;

    /**
     * @brief Gets all book moves for a position (for analysis)
     *
     * @param board Current board state
     * @param player Player to move
     * @return Vector of book moves with statistics
     */
    std::vector<BookMove> getMoves(const Board_t& board, PlayerColor_t player) const;

    /**
     * @brief Clears the entire book
     */
    void clear();

    /**
     * @brief Prints book statistics
     */
    void printStats() const;

    // Getters
    int getTotalGames() const {
        return totalGamesLoaded;
    }
    int getTotalPositions() const {
        return totalPositions;
    }
    int getMaxDepth() const {
        return maxDepthStored;
    }
};

#endif  // OPENING_BOOK_H
