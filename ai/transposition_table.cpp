/**
 * @brief Transposition Table for Reversi AI
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "transposition_table.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <random>

// ============================================================================
// Constructor / Destructor
// ============================================================================

TranspositionTable::TranspositionTable() {
    size = TT_ENTRIES;
    table = new TTEntry[size];
    currentAge = 0;
    hits = 0;
    misses = 0;
    collisions = 0;

    initZobrist();
    clear();

    std::cout << "Transposition Table initialized: " << TT_SIZE_MB << " MB (" << size << " entries)"
              << std::endl;
}

TranspositionTable::~TranspositionTable() {
    delete[] table;
}

// ============================================================================
// Initialization
// ============================================================================

void TranspositionTable::initZobrist() {
    // Use fixed seed for reproducibility (change for variety)
    std::mt19937_64 rng(0x123456789ABCDEFULL);
    std::uniform_int_distribution<uint64_t> dist;

    // Initialize piece hashes
    for (int player = 0; player < 2; player++) {
        for (int pos = 0; pos < 64; pos++) {
            zobristPieces[player][pos] = dist(rng);
        }
    }

    // Player to move hash
    zobristPlayer = dist(rng);
}

void TranspositionTable::clear() {
    memset(table, 0, size * sizeof(TTEntry));
    currentAge = 0;
    hits = 0;
    misses = 0;
    collisions = 0;
}

void TranspositionTable::newSearch() {
    currentAge++;
    if (currentAge == 0) {
        // Age wrapped around (after 256 searches)
        currentAge = 1;
    }
}

// ============================================================================
// Hashing
// ============================================================================

size_t TranspositionTable::getIndex(uint64_t hash) const {
    // Use modulo for index (fast with power-of-2 sizes)
    return hash % size;
}

uint64_t TranspositionTable::computeHash(const Board_t& board, PlayerColor_t player) const {
    uint64_t hash = 0;

    // Hash black pieces
    uint64_t blackBB = board.black;
    while (blackBB) {
        Move_t pos = bitScanForward(blackBB);
        hash ^= zobristPieces[PLAYER_BLACK][pos];
        blackBB &= blackBB - 1;  // Clear LSB
    }

    // Hash white pieces
    uint64_t whiteBB = board.white;
    while (whiteBB) {
        Move_t pos = bitScanForward(whiteBB);
        hash ^= zobristPieces[PLAYER_WHITE][pos];
        whiteBB &= whiteBB - 1;  // Clear LSB
    }

    // Hash player to move
    if (player == PLAYER_BLACK) {
        hash ^= zobristPlayer;
    }

    return hash;
}

uint64_t TranspositionTable::updateHash(uint64_t hash,
                                        Move_t move,
                                        uint64_t flips,
                                        PlayerColor_t player) const {
    // Remove old player piece (if overwriting)
    // Add new player piece at move position
    hash ^= zobristPieces[player][move];

    // Flip opponent pieces
    PlayerColor_t opponent = getOpponent(player);
    while (flips) {
        Move_t pos = bitScanForward(flips);
        hash ^= zobristPieces[opponent][pos];  // Remove opponent piece
        hash ^= zobristPieces[player][pos];    // Add player piece
        flips &= flips - 1;
    }

    // Switch player to move
    hash ^= zobristPlayer;

    return hash;
}

// ============================================================================
// Probe and Store
// ============================================================================

bool TranspositionTable::probe(
    uint64_t hash, int depth, int alpha, int beta, int& score, Move_t& bestMove) {
    size_t index = getIndex(hash);
    TTEntry& entry = table[index];

    // Check if entry matches this position
    if (entry.zobristKey != hash) {
        misses++;
        return false;
    }

    // Entry found
    hits++;

    // Always return best move if available
    if (entry.bestMove != MOVE_NONE) {
        bestMove = entry.bestMove;
    }

    // Check if depth is sufficient
    if (entry.depth < depth) {
        return false;  // Not deep enough, can't use score
    }

    // Check bound type and validate score
    int storedScore = entry.score;

    if (entry.bound == BOUND_EXACT) {
        // Exact score - can use directly
        score = storedScore;
        return true;
    } else if (entry.bound == BOUND_LOWER) {
        // Lower bound (failed high / beta cutoff)
        if (storedScore >= beta) {
            score = storedScore;
            return true;
        }
    } else if (entry.bound == BOUND_UPPER) {
        // Upper bound (failed low / alpha cutoff)
        if (storedScore <= alpha) {
            score = storedScore;
            return true;
        }
    }

    return false;  // Score not usable
}

void TranspositionTable::store(uint64_t hash, int depth, int score, int bound, Move_t bestMove) {
    size_t index = getIndex(hash);
    TTEntry& entry = table[index];

    // Replacement scheme: prefer deeper, newer entries
    bool replace = false;

    if (entry.zobristKey == 0) {
        // Empty slot
        replace = true;
    } else if (entry.zobristKey == hash) {
        // Same position - replace if deeper or same depth
        replace = (depth >= entry.depth);
    } else {
        // Collision - use replacement strategy
        collisions++;

        // Replace if:
        // 1. Current entry is old (different generation)
        // 2. OR new entry is significantly deeper
        if (entry.age != currentAge) {
            replace = true;
        } else if (depth > entry.depth + 2) {
            replace = true;
        }
    }

    if (replace) {
        entry.zobristKey = hash;
        entry.depth = depth;
        entry.score = score;
        entry.bound = bound;
        entry.bestMove = bestMove;
        entry.age = currentAge;
    }
}

Move_t TranspositionTable::getBestMove(uint64_t hash) const {
    size_t index = getIndex(hash);
    const TTEntry& entry = table[index];

    if (entry.zobristKey == hash) {
        return entry.bestMove;
    }

    return MOVE_NONE;
}

void TranspositionTable::prefetch(uint64_t hash) const {
    size_t index = getIndex(hash);
#if defined(__GNUC__) || defined(__clang__)
    __builtin_prefetch(&table[index], 0, 3);
#elif defined(_MSC_VER)
    // Visual Studio: use _mm_prefetch if available (SSE required)
    #include <xmmintrin.h>
    _mm_prefetch(reinterpret_cast<const char*>(&table[index]), _MM_HINT_T0);
#else
    // No prefetch available; do nothing
    (void)index;
#endif
}

// ============================================================================
// Statistics
// ============================================================================

void TranspositionTable::printStats() const {
    uint64_t total = hits + misses;

    std::cout << "\n=== Transposition Table Statistics ===" << std::endl;
    std::cout << "Size: " << TT_SIZE_MB << " MB (" << size << " entries)" << std::endl;
    std::cout << "Lookups: " << total << std::endl;
    std::cout << "Hits: " << hits << " (" << (getHitRate() * 100.0) << "%)" << std::endl;
    std::cout << "Misses: " << misses << std::endl;
    std::cout << "Collisions: " << collisions << std::endl;
    std::cout << "Occupancy: " << (getOccupancy() * 100.0) << "%" << std::endl;
    std::cout << "======================================\n" << std::endl;
}

double TranspositionTable::getOccupancy() const {
    // Sample 1000 entries to estimate occupancy
    const size_t sampleSize = std::min((size_t)1000, size);
    size_t occupied = 0;

    for (size_t i = 0; i < sampleSize; i++) {
        size_t index = (i * size) / sampleSize;  // Evenly distributed samples
        if (table[index].zobristKey != 0) {
            occupied++;
        }
    }

    return (double)occupied / sampleSize;
}
