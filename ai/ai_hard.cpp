/**
 * @brief Hard difficulty AI - Minimax with Alpha-Beta pruning
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "ai_hard.h"
#include <algorithm>
#include <limits>
#include <iostream>

 // Positional weights: corners are highly valuable, edges are good, center is neutral
const int AIHard::POSITION_WEIGHTS[64] = {
    500,  -150, 30, 10, 10, 30, -150, 500,
    -150, -250, 0,  0,  0,  0,  -250, -150,
    30,   0,    1,  2,  2,  1,  0,    30,
    10,   0,    2,  16, 16, 2,  0,    10,
    10,   0,    2,  16, 16, 2,  0,    10,
    30,   0,    1,  2,  2,  1,  0,    30,
    -150, -250, 0,  0,  0,  0,  -250, -150,
    500,  -150, 30, 10, 10, 30, -150, 500
};

AIHard::AIHard() : nodesExplored(0), maxNodes(DEFAULT_NODE_LIMIT) {
    std::cout << "[AIHard] Initialized with node limit: " << maxNodes << std::endl;
}

GameModel AIHard::copyModel(const GameModel& model) const {
    return model;
}

int AIHard::evaluateBoard(const GameModel& model, PlayerColor_t maximizingPlayer) const {
    // Terminal position evaluation
    if (model.gameOver) {
        int myScore = countBits(getPlayerBitboard(model.board, maximizingPlayer));
        int oppScore = countBits(getOpponentBitboard(model.board, maximizingPlayer));

        if (myScore > oppScore) return 100000;
        if (myScore < oppScore) return -100000;
        return 0;
    }

    int score = 0;
    uint64_t myBoard = getPlayerBitboard(model.board, maximizingPlayer);
    uint64_t oppBoard = getOpponentBitboard(model.board, maximizingPlayer);

    // Positional evaluation using weight table
    for (int i = 0; i < 64; i++) {
        if (GET_BIT(myBoard, i))
            score += POSITION_WEIGHTS[i];
        if (GET_BIT(oppBoard, i))
            score -= POSITION_WEIGHTS[i];
    }

    // Mobility evaluation - difference in available moves
    int myMobility = getMoveCount(model.board, maximizingPlayer);
    int oppMobility = getMoveCount(model.board, getOpponent(maximizingPlayer));

    int totalPieces = countBits(myBoard | oppBoard);

    // Mobility importance varies by game phase
    if (totalPieces < 50) {
        score += (myMobility - oppMobility) * 80;
    }
    else {
        score += (myMobility - oppMobility) * 30;
    }

    // Piece count becomes crucial in endgame
    if (totalPieces > 50) {
        int myPieces = countBits(myBoard);
        int oppPieces = countBits(oppBoard);
        score += (myPieces - oppPieces) * 150;
    }

    // Corner control bonus
    score += countRegion(model.board, maximizingPlayer, CORNERS) * 500;
    score -= countRegion(model.board, getOpponent(maximizingPlayer), CORNERS) * 500;

    return score;
}

int AIHard::alphaBeta(GameModel& model, int depth, int alpha, int beta,
    bool isMaximizing, PlayerColor_t maximizingPlayer) const {
    nodesExplored++;

    // Check node limit (nuevo)
    if (nodesExplored >= maxNodes) {
        return evaluateBoard(model, maximizingPlayer);
    }

    // Search termination conditions
    if (depth == 0 || model.gameOver) {
        return evaluateBoard(model, maximizingPlayer);
    }

    MoveList validMoves;
    getValidMoves(model, validMoves);

    // Handle pass moves
    if (validMoves.empty()) {
        GameModel nextModel = copyModel(model);
        nextModel.currentPlayer = getOpponent(nextModel.currentPlayer);

        MoveList oppMoves;
        getValidMoves(nextModel, oppMoves);

        if (oppMoves.empty()) {
            nextModel.gameOver = true;
            return evaluateBoard(nextModel, maximizingPlayer);
        }

        // Opponent gets to move
        return alphaBeta(nextModel, depth - 1, alpha, beta, !isMaximizing, maximizingPlayer);
    }

    // Move ordering for better pruning - corners first
    std::sort(validMoves.begin(), validMoves.end(), [this](Move_t a, Move_t b) {
        return POSITION_WEIGHTS[a] > POSITION_WEIGHTS[b];
        });

    if (isMaximizing) {
        int maxEval = std::numeric_limits<int>::min();

        for (Move_t move : validMoves) {
            if (nodesExplored >= maxNodes) break;  // Early exit

            GameModel nextModel = copyModel(model);
            playMove(nextModel, move);

            int eval = alphaBeta(nextModel, depth - 1, alpha, beta, false, maximizingPlayer);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);

            // Beta pruning - opponent won't allow this path
            if (beta <= alpha) {
                break;
            }
        }

        return maxEval;
    }
    else {
        int minEval = std::numeric_limits<int>::max();

        for (Move_t move : validMoves) {
            if (nodesExplored >= maxNodes) break;  // Early exit

            GameModel nextModel = copyModel(model);
            playMove(nextModel, move);

            int eval = alphaBeta(nextModel, depth - 1, alpha, beta, true, maximizingPlayer);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);

            // Alpha pruning - we won't choose this path
            if (beta <= alpha) {
                break;
            }
        }

        return minEval;
    }
}

Move_t AIHard::getBestMove(GameModel& model) {
    nodesExplored = 0;

    MoveList validMoves;
    getValidMoves(model, validMoves);

    if (validMoves.empty()) {
        return MOVE_NONE;
    }

    // Dynamic depth adjustment based on game phase
    int totalPieces = getDiscCount(model.board);
    int searchDepth = MAX_DEPTH;

    if (totalPieces > 52) {
        // Endgame - search deeper when fewer moves remain
        searchDepth = std::min(64 - totalPieces, 15);
    }
    else if (totalPieces < 20) {
        // Opening - shallower search due to higher branching
        searchDepth = MAX_DEPTH - 2;
    }

    PlayerColor_t currentPlayer = getCurrentPlayer(model);
    Move_t bestMove = validMoves[0];
    int bestScore = std::numeric_limits<int>::min();

    std::cout << "[AIHard] Searching depth " << searchDepth
        << " (node limit: " << maxNodes << ")..." << std::endl;

    // Evaluate each move with alpha-beta search
    for (Move_t move : validMoves) {
        if (nodesExplored >= maxNodes) {
            std::cout << "[AIHard] Node limit reached, stopping early" << std::endl;
            break;
        }

        GameModel nextModel = copyModel(model);
        playMove(nextModel, move);

        int score = alphaBeta(nextModel, searchDepth - 1,
            std::numeric_limits<int>::min(),
            std::numeric_limits<int>::max(),
            false, currentPlayer);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    std::cout << "[AIHard] Best move: " << (int)bestMove
        << " (score: " << bestScore
        << ", nodes: " << nodesExplored << ")" << std::endl;

    return bestMove;
}