/**
 * @brief Opening Book Implementation
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "opening_book.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

namespace fs = std::filesystem;

// ============================================================================
// Constructor
// ============================================================================

OpeningBook::OpeningBook(TranspositionTable* transpositionTable)
    : tt(transpositionTable), totalGamesLoaded(0), totalPositions(0), maxDepthStored(0) {
}

// ============================================================================
// WThor Move Decoding
// ============================================================================

Move_t OpeningBook::decodeWThorMove(uint8_t wthorMove) const {
    if (wthorMove == 0)
        return MOVE_NONE;  // No move (game ended)

    // WThor encoding: (row * 10 + col + 10)
    // Decode: subtract 10, then extract row/col
    int decoded = wthorMove - 10;
    int row = decoded / 10;
    int col = decoded % 10;

    // Validate
    if (row < 0 || row >= 8 || col < 0 || col >= 8) {
        return MOVE_NONE;
    }

    // Convert to our format: row * 8 + col
    return coordsToMove(col, row);
}

// ============================================================================
// Game Loading
// ============================================================================

void OpeningBook::addGame(const std::vector<Move_t>& moves, int blackScore) {
    if (moves.empty())
        return;

    // Determine game outcome from black's perspective
    int whiteScore = 64 - blackScore;
    bool blackWon = blackScore > whiteScore;
    bool draw = blackScore == whiteScore;

    // Play through the game and add positions to book
    Board_t board;
    board.black = 0;
    board.white = 0;

    // Set up initial position
    SET_BIT(board.black, coordsToMove(3, 4));  // d5
    SET_BIT(board.black, coordsToMove(4, 3));  // e4
    SET_BIT(board.white, coordsToMove(3, 3));  // d4
    SET_BIT(board.white, coordsToMove(4, 4));  // e5

    PlayerColor_t player = PLAYER_BLACK;

    // Add up to BOOK_MAX_DEPTH moves
    int depth = std::min((int)moves.size(), BOOK_MAX_DEPTH);
    maxDepthStored = std::max(maxDepthStored, depth);

    for (int i = 0; i < depth; i++) {
        Move_t move = moves[i];
        if (move == MOVE_NONE)
            break;

        // Compute hash for current position
        uint64_t hash = tt->computeHash(board, player);

        // Get or create book position
        BookPosition& pos = book[hash];
        pos.totalGames++;

        // Find or add this move
        BookMove* bookMove = nullptr;
        for (auto& bm : pos.moves) {
            if (bm.move == move) {
                bookMove = &bm;
                break;
            }
        }

        if (!bookMove) {
            pos.moves.push_back(BookMove());
            bookMove = &pos.moves.back();
            bookMove->move = move;
        }

        // Update statistics based on game outcome
        bookMove->gameCount++;

        // Outcome from perspective of player who made this move
        bool playerIsBlack = (player == PLAYER_BLACK);
        bool playerWon = (playerIsBlack && blackWon) || (!playerIsBlack && !blackWon && !draw);

        if (draw) {
            bookMove->drawCount++;
        } else if (playerWon) {
            bookMove->winCount++;
        } else {
            bookMove->lossCount++;
        }

        // Make the move
        uint64_t playerBB = getPlayerBitboard(board, player);
        uint64_t opponentBB = getOpponentBitboard(board, player);
        uint64_t flips = calculateFlips(playerBB, opponentBB, move);

        uint64_t moveBit = 1ULL << move;
        if (player == PLAYER_BLACK) {
            board.black |= moveBit | flips;
            board.white &= ~flips;
        } else {
            board.white |= moveBit | flips;
            board.black &= ~flips;
        }

        player = getOpponent(player);
    }
}

int OpeningBook::loadWTBFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open book file: " << filename << std::endl;
        return 0;
    }

    // Read header (16 bytes)
    uint8_t header[16];
    file.read(reinterpret_cast<char*>(header), 16);

    if (!file) {
        std::cerr << "Failed to read header from: " << filename << std::endl;
        return 0;
    }

    // Extract game count (bytes 4-7, little-endian)
    int gameCount = header[4] | (header[5] << 8) | (header[6] << 16) | (header[7] << 24);

    std::cout << "Loading " << gameCount << " games from " << filename << "..." << std::endl;

    int gamesLoaded = 0;

    // Read each game (68 bytes per game)
    for (int g = 0; g < gameCount; g++) {
        uint8_t gameData[68];
        file.read(reinterpret_cast<char*>(gameData), 68);

        if (!file)
            break;

        // Extract moves (bytes 8-67, 60 moves max)
        std::vector<Move_t> moves;
        for (int i = 8; i < 68; i++) {
            Move_t move = decodeWThorMove(gameData[i]);
            if (move == MOVE_NONE)
                break;
            moves.push_back(move);
        }

        // Extract theoretical score (byte 7)
        int blackScore = gameData[7];

        // Validate score
        if (blackScore >= 0 && blackScore <= 64 && !moves.empty()) {
            addGame(moves, blackScore);
            gamesLoaded++;
        }
    }

    file.close();

    std::cout << "Loaded " << gamesLoaded << " games successfully." << std::endl;

    return gamesLoaded;
}

// ============================================================================
// Public Loading Functions
// ============================================================================

int OpeningBook::load(const std::string& directory) {
    int totalLoaded = 0;

    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.path().extension() == ".wtb") {
                totalLoaded += loadWTBFile(entry.path().string());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading directory: " << e.what() << std::endl;
    }

    totalGamesLoaded = totalLoaded;
    totalPositions = book.size();

    printStats();

    return totalLoaded;
}

int OpeningBook::loadFile(const std::string& filename) {
    int loaded = loadWTBFile(filename);
    totalGamesLoaded += loaded;
    totalPositions = book.size();

    printStats();

    return loaded;
}

// ============================================================================
// Book Querying
// ============================================================================

Move_t OpeningBook::probe(const Board_t& board, PlayerColor_t player, int moveCount) {
    // Don't use book beyond max depth
    if (moveCount > BOOK_MAX_DEPTH) {
        return MOVE_NONE;
    }

    uint64_t hash = tt->computeHash(board, player);

    auto it = book.find(hash);
    if (it == book.end()) {
        return MOVE_NONE;  // Position not in book
    }

    const BookPosition& pos = it->second;

    if (pos.moves.empty()) {
        return MOVE_NONE;
    }

    // Filter moves by minimum game count
    std::vector<BookMove> candidates;
    for (const auto& bm : pos.moves) {
        if (bm.gameCount >= BOOK_MIN_GAME_COUNT) {
            candidates.push_back(bm);
        }
    }

    if (candidates.empty()) {
        return MOVE_NONE;
    }

    // Sort by score (win rate + popularity)
    std::sort(candidates.begin(), candidates.end(), [](const BookMove& a, const BookMove& b) {
        return a.getScore() > b.getScore();
    });

    // Add some randomness to avoid predictability
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    // 85% pick best move, 15% pick second best (if available)
    if (candidates.size() > 1 && dist(rng) < BOOK_RANDOMNESS) {
        return candidates[1].move;
    }

    return candidates[0].move;
}

bool OpeningBook::contains(const Board_t& board, PlayerColor_t player) const {
    uint64_t hash = tt->computeHash(board, player);
    return book.find(hash) != book.end();
}

std::vector<BookMove> OpeningBook::getMoves(const Board_t& board, PlayerColor_t player) const {
    uint64_t hash = tt->computeHash(board, player);

    auto it = book.find(hash);
    if (it == book.end()) {
        return {};
    }

    return it->second.moves;
}

// ============================================================================
// Utilities
// ============================================================================

void OpeningBook::clear() {
    book.clear();
    totalGamesLoaded = 0;
    totalPositions = 0;
    maxDepthStored = 0;
}

void OpeningBook::printStats() const {
    std::cout << "\n=== Opening Book Statistics ===" << std::endl;
    std::cout << "Games loaded: " << totalGamesLoaded << std::endl;
    std::cout << "Unique positions: " << totalPositions << std::endl;
    std::cout << "Max depth stored: " << maxDepthStored << std::endl;
    std::cout << "Memory usage: ~" << (book.size() * 100 / 1024) << " KB" << std::endl;
    std::cout << "================================\n" << std::endl;
}
