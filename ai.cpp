/**
 * @brief Implements the Reversi game AI
 * @author Marc S. Ressl
 * @modified:
 *			Agustin Valenzuela,
 *			Alex Petersen,
 *			Dylan Frigerio,
 *			Enzo Fernadez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "ai.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

#include "controller.h"
#include "view.h"

#if defined(HARD_DIFFICULTY)
namespace {
// Adjustable configuration
const int MAX_DEPTH = 8;
const int MAX_NODES = 500000;

int nodesExplored = 0;

// Positional value table (weight of each square)
const int POSITION_WEIGHTS[64] = {
    500,  -150, 30, 10, 10, 30, -150, 500,  -150, -250, 0,  0,  0,  0,  -250, -150,
    30,   0,    1,  2,  2,  1,  0,    30,   10,   0,    2,  16, 16, 2,  0,    10,
    10,   0,    2,  16, 16, 2,  0,    10,   30,   0,    1,  2,  2,  1,  0,    30,
    -150, -250, 0,  0,  0,  0,  -250, -150, 500,  -150, 30, 10, 10, 30, -150, 500};

/**
 * @brief Copies the game state for simulation
 */
inline GameModel copyModel(const GameModel& model) {
    GameModel copy = model;
    return copy;
}

/**
 * @brief Enhanced evaluation function using positional weights
 */
int evaluateBoard(const GameModel& model, PlayerColor_t maximizingPlayer) {
    if (model.gameOver) {
        int myScore = countBits(getPlayerBitboard(model.board, maximizingPlayer));
        int oppScore = countBits(getOpponentBitboard(model.board, maximizingPlayer));

        if (myScore > oppScore)
            return 100000;
        if (myScore < oppScore)
            return -100000;
        return 0;
    }

    int score = 0;
    uint64_t myBoard = getPlayerBitboard(model.board, maximizingPlayer);
    uint64_t oppBoard = getOpponentBitboard(model.board, maximizingPlayer);

    // 1. Positional weights
    for (int i = 0; i < 64; i++) {
        if (GET_BIT(myBoard, i))
            score += POSITION_WEIGHTS[i];
        if (GET_BIT(oppBoard, i))
            score -= POSITION_WEIGHTS[i];
    }

    // 2. Mobility
    int myMobility = getMoveCount(model.board, maximizingPlayer);
    int oppMobility = getMoveCount(model.board, getOpponent(maximizingPlayer));

    int totalPieces = countBits(myBoard | oppBoard);

    // Mobility is more important in the midgame
    if (totalPieces < 50) {
        score += (myMobility - oppMobility) * 80;
    } else {
        score += (myMobility - oppMobility) * 30;
    }

    // 3. Piece parity
    if (totalPieces > 50) {
        int myPieces = countBits(myBoard);
        int oppPieces = countBits(oppBoard);
        score += (myPieces - oppPieces) * 150;
    }

    // 4. Stability
    score += countRegion(model.board, maximizingPlayer, CORNERS) * 500;
    score -= countRegion(model.board, getOpponent(maximizingPlayer), CORNERS) * 500;

    return score;
}

/**
 * @brief Minimax algorithm with Alpha-Beta pruning
 */
int alphaBeta(GameModel& model,
              int depth,
              int alpha,
              int beta,
              bool isMaximizing,
              PlayerColor_t maximizingPlayer) {
    nodesExplored++;

    if (depth == 0 || model.gameOver || nodesExplored >= MAX_NODES) {
        return evaluateBoard(model, maximizingPlayer);
    }

    MoveList validMoves;
    getValidMoves(model, validMoves);

    // If the current player has no moves available
    if (validMoves.empty()) {
        GameModel nextModel = copyModel(model);
        nextModel.currentPlayer = getOpponent(nextModel.currentPlayer);

        MoveList oppMoves;
        getValidMoves(nextModel, oppMoves);
        if (oppMoves.empty()) {
            nextModel.gameOver = true;
            return evaluateBoard(nextModel, maximizingPlayer);
        }

        return alphaBeta(nextModel, depth - 1, alpha, beta, !isMaximizing, maximizingPlayer);
    }

    // Move ordering
    std::sort(validMoves.begin(), validMoves.end(), [](Move_t a, Move_t b) {
        return POSITION_WEIGHTS[a] > POSITION_WEIGHTS[b];
    });

    if (isMaximizing) {
        int maxEval = std::numeric_limits<int>::min();

        for (Move_t move : validMoves) {
            GameModel nextModel = copyModel(model);
            playMove(nextModel, move);

            int eval = alphaBeta(nextModel, depth - 1, alpha, beta, false, maximizingPlayer);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);

            if (beta <= alpha) {
                break;  // Beta cutoff
            }
        }

        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();

        for (Move_t move : validMoves) {
            GameModel nextModel = copyModel(model);
            playMove(nextModel, move);

            int eval = alphaBeta(nextModel, depth - 1, alpha, beta, true, maximizingPlayer);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);

            if (beta <= alpha) {
                break;  // Alpha cutoff
            }
        }

        return minEval;
    }
}
}  // namespace

/**
 * @brief Selects the best move using minimax search with alpha-beta pruning
 */
Move_t getBestMove(GameModel& model) {
    nodesExplored = 0;

    MoveList validMoves;
    getValidMoves(model, validMoves);

    if (validMoves.empty()) {
        return MOVE_NONE;
    }

    // Dynamically adjust search depth based on game phase
    int totalPieces = getDiscCount(model.board);
    int searchDepth = MAX_DEPTH;

    if (totalPieces > 52) {
        // Endgame: perform full search if possible
        searchDepth = std::min(64 - totalPieces, 15);
    } else if (totalPieces < 20) {
        // Opening: shallower search
        searchDepth = MAX_DEPTH - 2;
    }

    PlayerColor_t currentPlayer = getCurrentPlayer(model);
    Move_t bestMove = validMoves[0];
    int bestScore = std::numeric_limits<int>::min();

    for (Move_t move : validMoves) {
        GameModel nextModel = copyModel(model);
        playMove(nextModel, move);

        int score = alphaBeta(nextModel,
                              searchDepth - 1,
                              std::numeric_limits<int>::min(),
                              std::numeric_limits<int>::max(),
                              false,
                              currentPlayer);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    return bestMove;
}
#elif defined(EXTREME_DIFFICULTY)

// ============================================================================
// Evaluator Implementation
// ============================================================================

// Piece-square table: positional values for each square
// Corners are most valuable, X-squares are dangerous, edges are good
const int Evaluator::pieceSquareTable[64] = {
    100, -20, 10, 5,  5,  10, -20, 100,  // Rank 1
    -20, -50, -2, -2, -2, -2, -50, -20,  // Rank 2
    10,  -2,  5,  1,  1,  5,  -2,  10,   // Rank 3
    5,   -2,  1,  1,  1,  1,  -2,  5,    // Rank 4
    5,   -2,  1,  1,  1,  1,  -2,  5,    // Rank 5
    10,  -2,  5,  1,  1,  5,  -2,  10,   // Rank 6
    -20, -50, -2, -2, -2, -2, -50, -20,  // Rank 7
    100, -20, 10, 5,  5,  10, -20, 100   // Rank 8
};

int Evaluator::evaluate(const Board_t& board, PlayerColor_t player) {
    int emptyCount = getEmptyCount(board);

    // In endgame, disc count matters most
    if (emptyCount <= 10) {
        return evaluateDiscParity(board, player) * 10;
    }

    // Combine multiple heuristics
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

int Evaluator::evaluateMobility(const Board_t& board, PlayerColor_t player) {
    int myMoves = getMoveCount(board, player);
    int oppMoves = getMoveCount(board, getOpponent(player));

    // Avoid division by zero
    if (myMoves + oppMoves == 0)
        return 0;

    return myMoves - oppMoves;
}

int Evaluator::evaluateCorners(const Board_t& board, PlayerColor_t player) {
    int myCorners = getCornerCount(board, player);
    int oppCorners = getCornerCount(board, getOpponent(player));

    return myCorners - oppCorners;
}

int Evaluator::evaluatePositional(const Board_t& board, PlayerColor_t player) {
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

int Evaluator::evaluateStability(const Board_t& board, PlayerColor_t player) {
    // Simplified stability: corners and edges adjacent to corners
    // Full stability analysis is complex and will be added in Phase 2b

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

int Evaluator::evaluateFrontier(const Board_t& board, PlayerColor_t player) {
    // Frontier discs are pieces adjacent to empty squares
    // They are vulnerable to being flipped

    uint64_t myPieces = getPlayerBitboard(board, player);
    uint64_t oppPieces = getOpponentBitboard(board, player);
    uint64_t empty = getEmptyBitboard(board);

    // Find pieces adjacent to empty squares
    uint64_t adjacentToEmpty = 0;
    adjacentToEmpty |= (empty >> 8) | (empty << 8);  // N, S
    adjacentToEmpty |=
        ((empty & ~0x0101010101010101ULL) >> 1) | ((empty & ~0x8080808080808080ULL) << 1);  // W, E
    adjacentToEmpty |= ((empty & ~0x0101010101010101ULL) >> 9) |
                       ((empty & ~0x8080808080808080ULL) << 9);  // NW, SE
    adjacentToEmpty |= ((empty & ~0x8080808080808080ULL) >> 7) |
                       ((empty & ~0x0101010101010101ULL) << 7);  // NE, SW

    int myFrontier = countBits(myPieces & adjacentToEmpty);
    int oppFrontier = countBits(oppPieces & adjacentToEmpty);

    // Fewer frontier discs is better
    return oppFrontier - myFrontier;
}

int Evaluator::evaluateDiscParity(const Board_t& board, PlayerColor_t player) {
    return getScoreDiff(board, player);
}

// ============================================================================
// SearchEngine Implementation
// ============================================================================

SearchEngine::SearchEngine() {
    nodesSearched = 0;
    cutoffs = 0;
    maxDepthReached = 0;
    pvMove = MOVE_NONE;
    timeLimit = TIME_LIMIT_MS / 1000.0;
}

bool SearchEngine::isTimeUp() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = currentTime - searchStartTime;
    return elapsed.count() >= timeLimit;
}

Move_t SearchEngine::search(Board_t& board, PlayerColor_t player, double timeLimitSeconds) {
    searchStartTime = std::chrono::high_resolution_clock::now();
    timeLimit = timeLimitSeconds;
    nodesSearched = 0;
    cutoffs = 0;
    maxDepthReached = 0;

    Move_t bestMove = MOVE_NONE;
    int emptyCount = getEmptyCount(board);

    // Determine maximum depth based on game phase
    int maxDepth = MAX_SEARCH_DEPTH;
    if (emptyCount <= ENDGAME_THRESHOLD) {
        maxDepth = ENDGAME_DEPTH;  // Search deeper in endgame
    }

    // Iterative deepening: search depth 1, 2, 3, ... until time runs out
    for (int depth = 1; depth <= maxDepth; depth++) {
        if (isTimeUp())
            break;

        Move_t currentBest = rootSearch(board, player, depth, -INFINITY_SCORE, INFINITY_SCORE);

        if (currentBest != MOVE_NONE) {
            bestMove = currentBest;
            pvMove = currentBest;  // Save for next iteration's move ordering
            maxDepthReached = depth;
        }

        // If we found a winning move, no need to search deeper
        if (bestMove != MOVE_NONE) {
            // Check if this is a guaranteed win
            // (score would be very high from negamax)
            break;
        }

        if (isTimeUp())
            break;
    }

    // Print search statistics
    std::cout << "Search complete: Depth=" << maxDepthReached << " Nodes=" << nodesSearched
              << " Cutoffs=" << cutoffs << std::endl;

    return bestMove;
}

Move_t SearchEngine::rootSearch(
    Board_t& board, PlayerColor_t player, int depth, int alpha, int beta) {
    MoveList moves;
    getValidMovesAI(board, player, moves);

    if (moves.empty())
        return MOVE_NONE;

    // Order moves for better pruning
    orderMoves(moves, board, player);

    Move_t bestMove = moves[0];
    int bestScore = -INFINITY_SCORE;

    for (Move_t move : moves) {
        if (isTimeUp())
            break;

        PlayerColor_t nextPlayer = player;
        BoardState_t state = makeMove(board, nextPlayer, move);

        int score = -negamax(board, nextPlayer, depth - 1, -beta, -alpha);

        unmakeMove(board, nextPlayer, state);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        alpha = std::max(alpha, score);
        if (alpha >= beta) {
            cutoffs++;
            break;  // Beta cutoff
        }
    }

    return bestMove;
}

int SearchEngine::negamax(Board_t& board, PlayerColor_t player, int depth, int alpha, int beta) {
    nodesSearched++;

    // Terminal node checks
    if (depth == 0 || isTerminal(board, player)) {
        return evaluator.evaluate(board, player);
    }

    // Time check every ~1000 nodes
    if ((nodesSearched & 0x3FF) == 0 && isTimeUp()) {
        return evaluator.evaluate(board, player);
    }

    MoveList moves;
    getValidMovesAI(board, player, moves);

    // If no moves, pass to opponent
    if (moves.empty()) {
        PlayerColor_t opponent = getOpponent(player);

        // Check if opponent can move
        if (!hasValidMoves(board, opponent)) {
            // Game over - return final score
            int finalScore = getScoreDiff(board, player);
            if (finalScore > 0)
                return WIN_SCORE;
            else if (finalScore < 0)
                return LOSE_SCORE;
            else
                return 0;
        }

        // Pass: opponent moves, then it's our turn again
        return -negamax(board, opponent, depth - 1, -beta, -alpha);
    }

    // Order moves
    orderMoves(moves, board, player);

    int bestScore = -INFINITY_SCORE;

    for (Move_t move : moves) {
        PlayerColor_t nextPlayer = player;
        BoardState_t state = makeMove(board, nextPlayer, move);

        int score = -negamax(board, nextPlayer, depth - 1, -beta, -alpha);

        unmakeMove(board, nextPlayer, state);

        bestScore = std::max(bestScore, score);
        alpha = std::max(alpha, score);

        if (alpha >= beta) {
            cutoffs++;
            break;  // Beta cutoff
        }
    }

    return bestScore;
}

void SearchEngine::orderMoves(MoveList& moves, const Board_t& board, PlayerColor_t player) {
    // Sort moves by heuristic score (descending)
    std::sort(moves.begin(), moves.end(), [this, &board, player](Move_t a, Move_t b) {
        // PV move first
        if (a == pvMove)
            return true;
        if (b == pvMove)
            return false;

        return scoreMoveForOrdering(a, board, player) > scoreMoveForOrdering(b, board, player);
    });
}

int SearchEngine::scoreMoveForOrdering(Move_t move, const Board_t& board, PlayerColor_t player) {
    int score = 0;

    // Corners are best
    if ((1ULL << move) & CORNERS) {
        score += 10000;
    }
    // X-squares are worst (give opponent corner access)
    else if ((1ULL << move) & X_SQUARES) {
        score -= 5000;
    }
    // Edges are good
    else if ((1ULL << move) & EDGES) {
        score += 100;
    }

    // Prefer moves that flip many discs
    uint64_t playerBB = getPlayerBitboard(board, player);
    uint64_t opponentBB = getOpponentBitboard(board, player);
    int flips = countBits(calculateFlips(playerBB, opponentBB, move));
    score += flips * 10;

    // Prefer moves that reduce opponent mobility
    Board_t testBoard = board;
    PlayerColor_t testPlayer = player;
    makeMove(testBoard, testPlayer, move);
    int oppMobility = getMoveCount(testBoard, testPlayer);
    score -= oppMobility * 5;

    return score;
}

int SearchEngine::quiescence(Board_t& board, PlayerColor_t player, int alpha, int beta) {
    // Quiescence search to avoid horizon effect
    // For now, just return static evaluation
    // Will be enhanced in Phase 2b
    return evaluator.evaluate(board, player);
}

int SearchEngine::getNodesSearched() const {
    return nodesSearched;
}

int SearchEngine::getCutoffs() const {
    return cutoffs;
}

int SearchEngine::getMaxDepthReached() const {
    return maxDepthReached;
}

// ============================================================================
// Main AI Interface
// ============================================================================

Move_t getBestMove(GameModel& model) {
    // Create search engine
    SearchEngine engine;

    // Make a copy of the board for search
    Board_t board = model.board;
    PlayerColor_t player = model.currentPlayer;

    // Get valid moves first (fallback to random if search fails)
    MoveList validMoves;
    getValidMovesAI(board, player, validMoves);

    if (validMoves.empty()) {
        return MOVE_NONE;
    }

    // If only one legal move, return it immediately
    if (validMoves.size() == 1) {
        std::cout << "Only one move available: " << (int)validMoves[0] << std::endl;
        return validMoves[0];
    }

    // Run search with time limit
    double timeLimit = TIME_LIMIT_MS / 1000.0;
    Move_t bestMove = engine.search(board, player, timeLimit);

    // Fallback to first valid move if search failed
    if (bestMove == MOVE_NONE) {
        bestMove = validMoves[0];
    }

    std::cout << "AI chooses move: " << (int)bestMove << " [" << (char)('A' + getMoveX(bestMove))
              << (getMoveY(bestMove) + 1) << "]" << std::endl;

    return bestMove;
}

#endif
