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

#include "ai_extreme.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <limits>

namespace fs = std::filesystem;

// ============================================================================
// Search Configuration Constants
// ============================================================================

#define MAX_SEARCH_DEPTH 12
#define TIME_LIMIT_MS 15000
#define ENDGAME_DEPTH 16
#define ENDGAME_THRESHOLD 12

// L�mite de nodos por defecto - ahora configurable en runtime
static const int DEFAULT_MAX_NODES = 500000;

// Book year limit to stop looking for in the database folder
#define BOOK_LIMIT_YEAR 1977

#define INFINITY_SCORE 1000000
#define WIN_SCORE 100000
#define LOSE_SCORE -100000

// Note: CORNERS, EDGES, X_SQUARES already defined in model.h

// Evaluation function weights
#define WEIGHT_MOBILITY 10
#define WEIGHT_CORNER 100
#define WEIGHT_X_SQUARE -50
#define WEIGHT_C_SQUARE -20
#define WEIGHT_EDGE 5
#define WEIGHT_STABILITY 15
#define WEIGHT_FRONTIER -5

// ============================================================================
// Evaluator Implementation
// ============================================================================

/**
 * @brief Advanced board evaluation with multiple heuristics
 */
class AIExtreme::Evaluator {
public:
    static const int pieceSquareTable[64];

    int evaluate(const Board_t& board, PlayerColor_t player) {
        int emptyCount = getEmptyCount(board);

        // In endgame, disc count matters most
        if (emptyCount <= 10) {
            return evaluateDiscParity(board, player) * 10;
        }

        int score = 0;

        // Mobility - very important in midgame
        score += evaluateMobility(board, player) * WEIGHT_MOBILITY;

        // Corner control - always important
        score += evaluateCorners(board, player) * WEIGHT_CORNER;

        // Positional evaluation
        score += evaluatePositional(board, player);

        // Stability - more important in late-midgame
        if (emptyCount < 30) {
            score += evaluateStability(board, player) * WEIGHT_STABILITY;
        }

        // Frontier discs - important in opening/midgame
        if (emptyCount > 20) {
            score += evaluateFrontier(board, player) * WEIGHT_FRONTIER;
        }

        // Disc parity - increasingly important as game progresses
        if (emptyCount < 20) {
            score += evaluateDiscParity(board, player) * (30 - emptyCount);
        }

        return score;
    }

    int evaluateMobility(const Board_t& board, PlayerColor_t player) {
        int myMoves = getMoveCount(board, player);
        int oppMoves = getMoveCount(board, getOpponent(player));

        if (myMoves + oppMoves == 0)
            return 0;

        return myMoves - oppMoves;
    }

    int evaluateCorners(const Board_t& board, PlayerColor_t player) {
        int myCorners = getCornerCount(board, player);
        int oppCorners = getCornerCount(board, getOpponent(player));

        return myCorners - oppCorners;
    }

    int evaluatePositional(const Board_t& board, PlayerColor_t player) {
        int score = 0;
        uint64_t myPieces = getPlayerBitboard(board, player);
        uint64_t oppPieces = getOpponentBitboard(board, player);

        for (int pos = 0; pos < 64; pos++) {
            if (myPieces & (1ULL << pos)) {
                score += pieceSquareTable[pos];
            }
            if (oppPieces & (1ULL << pos)) {
                score -= pieceSquareTable[pos];
            }
        }

        return score;
    }

    int evaluateStability(const Board_t& board, PlayerColor_t player) {
        int stableScore = 0;
        uint64_t myPieces = getPlayerBitboard(board, player);
        uint64_t oppPieces = getOpponentBitboard(board, player);

        // Corners are always stable
        stableScore += countBits(myPieces & CORNERS) * 5;
        stableScore -= countBits(oppPieces & CORNERS) * 5;

        // Edges are semi-stable
        stableScore += countBits(myPieces & EDGES);
        stableScore -= countBits(oppPieces & EDGES);

        return stableScore;
    }

    int evaluateFrontier(const Board_t& board, PlayerColor_t player) {
        uint64_t myPieces = getPlayerBitboard(board, player);
        uint64_t oppPieces = getOpponentBitboard(board, player);
        uint64_t empty = getEmptyBitboard(board);

        // Find pieces adjacent to empty squares
        uint64_t adjacentToEmpty = 0;
        adjacentToEmpty |= (empty >> 8) | (empty << 8);  // N, S
        adjacentToEmpty |= ((empty & ~0x0101010101010101ULL) >> 1) |
            ((empty & ~0x8080808080808080ULL) << 1);  // W, E
        adjacentToEmpty |= ((empty & ~0x0101010101010101ULL) >> 9) |
            ((empty & ~0x8080808080808080ULL) << 9);  // NW, SE
        adjacentToEmpty |= ((empty & ~0x8080808080808080ULL) >> 7) |
            ((empty & ~0x0101010101010101ULL) << 7);  // NE, SW

        int myFrontier = countBits(myPieces & adjacentToEmpty);
        int oppFrontier = countBits(oppPieces & adjacentToEmpty);

        return oppFrontier - myFrontier;
    }

    int evaluateDiscParity(const Board_t& board, PlayerColor_t player) {
        return getScoreDiff(board, player);
    }
};

// Piece-square table
const int AIExtreme::Evaluator::pieceSquareTable[64] = {
    100, -20, 10, 5,  5,  10, -20, 100, 
    -20, -50, -2, -2, -2, -2, -50, -20,
    10,  -2,  5,  1,  1,  5,  -2,  10,  
    5,   -2,  1,  1,  1,  1,  -2,  5,
    5,   -2,  1,  1,  1,  1,  -2,  5,   
    10,  -2,  5,  1,  1,  5,  -2,  10,
    -20, -50, -2, -2, -2, -2, -50, -20, 
    100, -20, 10, 5,  5,  10, -20, 100 };

// ============================================================================
// SearchEngine Implementation
// ============================================================================

class AIExtreme::SearchEngine {
public:
    TranspositionTable tt;  // Make public so OpeningBook can share it

    SearchEngine();
    Move_t search(Board_t& board, PlayerColor_t player, double timeLimitSeconds);

    int getNodesSearched() const {
        return nodesSearched;
    }
    int getMaxDepth() const {
        return maxDepthReached;
    }

	// New methods for node limit management
    void setMaxNodes(int limit) {
        maxNodesLimit = (limit > 0) ? limit : DEFAULT_MAX_NODES;
    }

    int getMaxNodes() const {
        return maxNodesLimit;
    }

private:
    Evaluator evaluator;

    int nodesSearched;
    int cutoffs;
    int maxDepthReached;
    Move_t pvMove;

    int maxNodesLimit;

    std::chrono::time_point<std::chrono::high_resolution_clock> searchStartTime;
    double timeLimit;

    bool isTimeUp();
    Move_t rootSearch(Board_t& board, PlayerColor_t player, int depth, int alpha, int beta);
    int negamax(
        Board_t& board, PlayerColor_t player, int depth, int alpha, int beta, uint64_t hash);
    void orderMoves(MoveList& moves, const Board_t& board, PlayerColor_t player);
    int scoreMoveForOrdering(Move_t move, const Board_t& board, PlayerColor_t player);
};

AIExtreme::SearchEngine::SearchEngine()
    : nodesSearched(0),
    cutoffs(0),
    maxDepthReached(0),
    pvMove(MOVE_NONE),
    maxNodesLimit(DEFAULT_MAX_NODES),
    timeLimit(TIME_LIMIT_MS / 1000.0) {
}

bool AIExtreme::SearchEngine::isTimeUp() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = currentTime - searchStartTime;
    return elapsed.count() >= timeLimit;
}

Move_t AIExtreme::SearchEngine::search(Board_t& board,
    PlayerColor_t player,
    double timeLimitSeconds) {
    searchStartTime = std::chrono::high_resolution_clock::now();
    timeLimit = timeLimitSeconds;
    nodesSearched = 0;
    cutoffs = 0;
    maxDepthReached = 0;

    tt.newSearch();

    Move_t bestMove = MOVE_NONE;
    int emptyCount = getEmptyCount(board);

    int maxDepth = MAX_SEARCH_DEPTH;
    if (emptyCount <= ENDGAME_THRESHOLD) {
        maxDepth = ENDGAME_DEPTH;
    }

    // Iterative deepening
    for (int depth = 1; depth <= maxDepth; depth++) {
        if (isTimeUp())
            break;

        Move_t currentBest = rootSearch(board, player, depth, -INFINITY_SCORE, INFINITY_SCORE);

        if (currentBest != MOVE_NONE) {
            bestMove = currentBest;
            pvMove = currentBest;
            maxDepthReached = depth;
        }

        if (isTimeUp())
            break;
    }

    std::cout << "Search complete: Depth=" << maxDepthReached << " Nodes=" << nodesSearched
        << " Cutoffs=" << cutoffs << " Limit=" << maxNodesLimit << std::endl;

    tt.printStats();

    return bestMove;
}

Move_t AIExtreme::SearchEngine::rootSearch(
    Board_t& board, PlayerColor_t player, int depth, int alpha, int beta) {
    MoveList moves;
    getValidMovesAI(board, player, moves);

    if (moves.empty())
        return MOVE_NONE;

    uint64_t hash = tt.computeHash(board, player);

    Move_t ttMove = tt.getBestMove(hash);
    if (ttMove != MOVE_NONE) {
        auto it = std::find(moves.begin(), moves.end(), ttMove);
        if (it != moves.end()) {
            moves.erase(it);
            moves.insert(moves.begin(), ttMove);
        }
    }

    orderMoves(moves, board, player);

    Move_t bestMove = moves[0];
    int bestScore = -INFINITY_SCORE;
    int bound = BOUND_UPPER;

    for (Move_t move : moves) {
        if (isTimeUp() || nodesSearched >= maxNodesLimit)
            break;

        PlayerColor_t nextPlayer = player;
        uint64_t playerBB = getPlayerBitboard(board, player);
        uint64_t opponentBB = getOpponentBitboard(board, player);
        uint64_t flips = calculateFlips(playerBB, opponentBB, move);

        BoardState_t state = makeMove(board, nextPlayer, move);
        uint64_t nextHash = tt.updateHash(hash, move, flips, player);

        int score = -negamax(board, nextPlayer, depth - 1, -beta, -alpha, nextHash);

        unmakeMove(board, nextPlayer, state);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        if (score > alpha) {
            alpha = score;
            bound = BOUND_EXACT;
        }

        if (alpha >= beta) {
            cutoffs++;
            bound = BOUND_LOWER;
            break;
        }
    }

    tt.store(hash, depth, bestScore, bound, bestMove);

    return bestMove;
}

int AIExtreme::SearchEngine::negamax(
    Board_t& board, PlayerColor_t player, int depth, int alpha, int beta, uint64_t hash) {
    nodesSearched++;

    int ttScore;
    Move_t ttMove = MOVE_NONE;
    if (tt.probe(hash, depth, alpha, beta, ttScore, ttMove)) {
        return ttScore;
    }

    if (depth == 0 || isTerminal(board, player)) {
        int score = evaluator.evaluate(board, player);
        tt.store(hash, depth, score, BOUND_EXACT, MOVE_NONE);
        return score;
    }

	// Verify node limit every 1024 nodes
    if ((nodesSearched & 0x3FF) == 0) {
        if (isTimeUp() || nodesSearched >= maxNodesLimit) {
            return evaluator.evaluate(board, player);
        }
    }

    MoveList moves;
    getValidMovesAI(board, player, moves);

    if (moves.empty()) {
        PlayerColor_t opponent = getOpponent(player);

        if (!hasValidMoves(board, opponent)) {
            int finalScore = getScoreDiff(board, player);
            int score;
            if (finalScore > 0)
                score = WIN_SCORE;
            else if (finalScore < 0)
                score = LOSE_SCORE;
            else
                score = 0;

            tt.store(hash, depth, score, BOUND_EXACT, MOVE_NONE);
            return score;
        }

        uint64_t passHash = hash ^ tt.getZobristPlayer();
        return -negamax(board, opponent, depth - 1, -beta, -alpha, passHash);
    }

    if (ttMove != MOVE_NONE) {
        auto it = std::find(moves.begin(), moves.end(), ttMove);
        if (it != moves.end()) {
            moves.erase(it);
            moves.insert(moves.begin(), ttMove);
        }
    }

    orderMoves(moves, board, player);

    int bestScore = -INFINITY_SCORE;
    Move_t bestMove = moves[0];
    int bound = BOUND_UPPER;

    for (Move_t move : moves) {

        if (nodesSearched >= maxNodesLimit)
            break;

        PlayerColor_t nextPlayer = player;
        uint64_t playerBB = getPlayerBitboard(board, player);
        uint64_t opponentBB = getOpponentBitboard(board, player);
        uint64_t flips = calculateFlips(playerBB, opponentBB, move);

        BoardState_t state = makeMove(board, nextPlayer, move);
        uint64_t nextHash = tt.updateHash(hash, move, flips, player);

        int score = -negamax(board, nextPlayer, depth - 1, -beta, -alpha, nextHash);

        unmakeMove(board, nextPlayer, state);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        if (score > alpha) {
            alpha = score;
            bound = BOUND_EXACT;
        }

        if (alpha >= beta) {
            cutoffs++;
            bound = BOUND_LOWER;
            bestScore = beta;
            break;
        }
    }

    tt.store(hash, depth, bestScore, bound, bestMove);

    return bestScore;
}

void AIExtreme::SearchEngine::orderMoves(MoveList& moves,
    const Board_t& board,
    PlayerColor_t player) {
    std::sort(moves.begin(), moves.end(), [this, &board, player](Move_t a, Move_t b) {
        if (a == pvMove)
            return true;
        if (b == pvMove)
            return false;

        return scoreMoveForOrdering(a, board, player) > scoreMoveForOrdering(b, board, player);
        });
}

int AIExtreme::SearchEngine::scoreMoveForOrdering(Move_t move,
    const Board_t& board,
    PlayerColor_t player) {
    int score = 0;

    if ((1ULL << move) & CORNERS) {
        score += 10000;
    }
    else if ((1ULL << move) & X_SQUARES) {
        score -= 5000;
    }
    else if ((1ULL << move) & EDGES) {
        score += 100;
    }

    uint64_t playerBB = getPlayerBitboard(board, player);
    uint64_t opponentBB = getOpponentBitboard(board, player);
    int flips = countBits(calculateFlips(playerBB, opponentBB, move));
    score += flips * 10;

    Board_t testBoard = board;
    PlayerColor_t testPlayer = player;
    makeMove(testBoard, testPlayer, move);
    int oppMobility = getMoveCount(testBoard, testPlayer);
    score -= oppMobility * 5;

    return score;
}

// ============================================================================
// AIExtreme Main Implementation
// ============================================================================

AIExtreme::AIExtreme() : moveCount(0) {
    engine = std::make_unique<SearchEngine>();
    book = std::make_unique<OpeningBook>(&engine->tt);  // Share TT with book

    int gamesLoaded = -1;
    std::string bookPath;

    bookPath.reserve(48);

    for (uint16_t year = 2024; year >= BOOK_LIMIT_YEAR && gamesLoaded != 0; year--) {
        std::format_to(std::back_inserter(bookPath), "./databases/WTH_{}.wtb", year);
        std::cout << "Loading opening book from: " << bookPath << std::endl;
        gamesLoaded = book->loadFile(bookPath);

        if (gamesLoaded == 0) {
            std::cerr << "Warning: Opening book not loaded. AI will use search for all moves."
                      << std::endl;
            std::cerr << "Expected file: " << bookPath << std::endl;
        }
        bookPath.clear();
    }
}

AIExtreme::~AIExtreme() = default;

int AIExtreme::loadOpeningBook(const std::string& path) {
    // Check if path is a file or directory
    if (fs::is_directory(path)) {
        return book->load(path);
    }
    else {
        return book->loadFile(path);
    }
}

Move_t AIExtreme::getBestMove(GameModel& model) {
    // Reset move counter if it's the start of a new game (4 pieces on board)
    int totalPieces = getDiscCount(model.board);

    // Detect new game: if very few pieces on board, it's early game
    if (totalPieces <= 6) {
        // This is early game - reset counter
        moveCount = 0;
    }

    Board_t board = model.board;
    PlayerColor_t player = model.currentPlayer;

    std::vector<Move_t> validMoves;
    getValidMovesAI(board, player, validMoves);

    if (validMoves.empty()) {
        return MOVE_NONE;
    }

    if (validMoves.size() == 1) {
        moveCount++;
        return validMoves[0];
    }

    // Try opening book first
    Move_t bookMove = book->probe(board, player, moveCount);
    if (bookMove != MOVE_NONE) {
        // Verify book move is legal
        auto it = std::find(validMoves.begin(), validMoves.end(), bookMove);
        if (it != validMoves.end()) {
            std::cout << "Opening book move: " << (int)bookMove << " ["
                << (char)('A' + getMoveX(bookMove)) << (getMoveY(bookMove) + 1) << "] (from "
                << book->getTotalGames() << " games)" << std::endl;
            moveCount++;
            return bookMove;
        }
    }

    // ONE-TIME TEST: Verify make/unmake works correctly
    static bool testedMakeUnmake = false;
    if (!testedMakeUnmake && !validMoves.empty()) {
        Board_t testBoard = board;
        PlayerColor_t testPlayer = player;
        Move_t testMove = validMoves[0];

        // Save original state
        uint64_t origBlack = testBoard.black;
        uint64_t origWhite = testBoard.white;
        PlayerColor_t origPlayer = testPlayer;

        // Make and unmake move
        BoardState_t state = makeMove(testBoard, testPlayer, testMove);
        unmakeMove(testBoard, testPlayer, state);

        // Verify restoration
        if (testBoard.black != origBlack || testBoard.white != origWhite ||
            testPlayer != origPlayer) {
            std::cerr << "\n=== CRITICAL ERROR: make/unmake is BROKEN! ===" << std::endl;
            std::cerr << "Original: black=" << origBlack << " white=" << origWhite
                      << " player=" << (int)origPlayer << std::endl;
            std::cerr << "After:    black=" << testBoard.black << " white=" << testBoard.white
                      << " player=" << (int)testPlayer << std::endl;
            std::cerr << "Move tested: " << (int)testMove << std::endl;
            std::cerr << "================================================\n" << std::endl;
        } else {
            std::cout << "[TEST] make/unmake verified OK ✓" << std::endl;
        }

        testedMakeUnmake = true;
    }

    // Not in book - use search
    double timeLimit = TIME_LIMIT_MS / 1000.0;
    Move_t bestMove = engine->search(board, player, timeLimit);

    if (bestMove == MOVE_NONE) {
        bestMove = validMoves[0];
    }

    moveCount++;
    return bestMove;
}

void AIExtreme::getSearchStats(int& nodesSearched, int& maxDepth) const {
    if (engine) {
        nodesSearched = engine->getNodesSearched();
        maxDepth = engine->getMaxDepth();
    }
    else {
        nodesSearched = 0;
        maxDepth = 0;
    }
}

// Implementation of new methods for node limit
void AIExtreme::setNodeLimit(int limit) {
    if (engine) {
        engine->setMaxNodes(limit);
        std::cout << "[AIExtreme] Node limit set to: " << limit << std::endl;
    }
}

int AIExtreme::getNodeLimit() const {
    if (engine) {
        return engine->getMaxNodes();
    }
    return DEFAULT_MAX_NODES;
}
