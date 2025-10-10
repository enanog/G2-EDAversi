#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <cstdint>

#include "model.h"

// ============================================================================
// Transposition Table Configuration
// ============================================================================

#define TT_SIZE_MB 256  // Table size in megabytes
#define TT_ENTRIES ((TT_SIZE_MB * 1024 * 1024) / sizeof(TTEntry))

// Bound types for alpha-beta scores
#define BOUND_EXACT 0  // Exact score (PV node)
#define BOUND_LOWER 1  // Alpha cutoff (>=)
#define BOUND_UPPER 2  // Beta cutoff (<=)

// ============================================================================
// Transposition Table Entry
// ============================================================================

/**
 * @brief Entry stored in the transposition table
 *
 * Size: 24 bytes (compact for cache efficiency)
 */
struct TTEntry {
    uint64_t zobristKey;  // 8 bytes - Position hash (for verification)
    int score;            // 4 bytes - Evaluation score
    Move_t bestMove;      // 1 byte  - Best move from this position
    int8_t depth;         // 1 byte  - Search depth
    uint8_t bound;        // 1 byte  - Bound type (EXACT/LOWER/UPPER)
    uint8_t age;          // 1 byte  - Generation counter (for replacement)
    // Padding: 8 bytes implicit

    TTEntry()
        : zobristKey(0), score(0), bestMove(MOVE_NONE), depth(-1), bound(BOUND_EXACT), age(0) {
    }
};

// ============================================================================
// Transposition Table Class
// ============================================================================

class TranspositionTable {
  private:
    TTEntry* table;      // Hash table
    size_t size;         // Number of entries
    uint8_t currentAge;  // Current generation

    // Zobrist hash tables (random numbers for hashing)
    uint64_t zobristPieces[2][64];  // [player][position]
    uint64_t zobristPlayer;         // XOR when black to move

    // Statistics
    uint64_t hits;        // TT hits
    uint64_t misses;      // TT misses
    uint64_t collisions;  // Hash collisions

    /**
     * @brief Gets table index from hash
     */
    size_t getIndex(uint64_t hash) const;

    /**
     * @brief Initializes Zobrist random numbers
     */
    void initZobrist();

  public:
    TranspositionTable();
    ~TranspositionTable();

    /**
     * @brief Clears the entire table
     */
    void clear();

    /**
     * @brief Increments age (call at start of each root search)
     */
    void newSearch();

    /**
     * @brief Computes Zobrist hash for a position
     *
     * @param board The board state
     * @param player Current player to move
     * @return 64-bit hash value
     */
    uint64_t computeHash(const Board_t& board, PlayerColor_t player) const;

    /**
     * @brief Incrementally updates hash after a move
     *
     * @param hash Current hash
     * @param move Move made
     * @param flips Bitboard of flipped pieces
     * @param player Player who made the move
     * @return Updated hash
     */
    uint64_t updateHash(uint64_t hash, Move_t move, uint64_t flips, PlayerColor_t player) const;

    /**
     * @brief Probes the transposition table
     *
     * @param hash Position hash
     * @param depth Current search depth
     * @param alpha Alpha bound
     * @param beta Beta bound
     * @param score Output: score if found
     * @param bestMove Output: best move if found
     * @return True if usable entry found
     */
    bool probe(uint64_t hash, int depth, int alpha, int beta, int& score, Move_t& bestMove);

    /**
     * @brief Stores position in transposition table
     *
     * @param hash Position hash
     * @param depth Search depth
     * @param score Evaluation score
     * @param bound Bound type (EXACT/LOWER/UPPER)
     * @param bestMove Best move from this position
     */
    void store(uint64_t hash, int depth, int score, int bound, Move_t bestMove);

    /**
     * @brief Gets best move from table without score validation
     *
     * @param hash Position hash
     * @return Best move, or MOVE_NONE if not found
     */
    Move_t getBestMove(uint64_t hash) const;

    /**
     * @brief Gets Zobrist player key (for pass moves)
     */
    uint64_t getZobristPlayer() const {
        return zobristPlayer;
    }

    /**
     * @brief Prefetches table entry (hint to CPU cache)
     *
     * @param hash Position hash
     */
    void prefetch(uint64_t hash) const;

    // Statistics getters
    uint64_t getHits() const {
        return hits;
    }
    uint64_t getMisses() const {
        return misses;
    }
    uint64_t getCollisions() const {
        return collisions;
    }
    double getHitRate() const {
        uint64_t total = hits + misses;
        return total > 0 ? (double)hits / total : 0.0;
    }

    /**
     * @brief Prints statistics
     */
    void printStats() const;

    /**
     * @brief Estimates table occupancy
     */
    double getOccupancy() const;
};

#endif
