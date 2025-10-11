/**
 * @brief Normal AI implementation
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "ai_normal.h"
#include <algorithm>
#include <limits>
#include <iostream>

AINormal::AINormal() : nodesExplored(0) {
    std::cout << "[AINormal] Initialized" << std::endl;
}

GameModel AINormal::copyModel(const GameModel& model) const {
    return model;
}

int AINormal::evaluateBoard(const GameModel& model, PlayerColor_t maximizingPlayer) const {
    // Simple evaluation: piece count difference
    if (model.gameOver) {
        int myScore = countBits(getPlayerBitboard(model.board, maximizingPlayer));
        int oppScore = countBits(getOpponentBitboard(model.board, maximizingPlayer));

        if (myScore > oppScore) return 10000;
        if (myScore < oppScore) return -10000;
        return 0;
    }

    int myPieces = countBits(getPlayerBitboard(model.board, maximizingPlayer));
    int oppPieces = countBits(getOpponentBitboard(model.board, maximizingPlayer));

    return myPieces - oppPieces;
}

int AINormal::minimax(GameModel& model, int depth, bool isMaximizing, PlayerColor_t maximizingPlayer) const {
    nodesExplored++;

    if (depth == 0 || model.gameOver) {
        return evaluateBoard(model, maximizingPlayer);
    }

    MoveList validMoves;
    getValidMoves(model, validMoves);

    if (validMoves.empty()) {
        GameModel nextModel = copyModel(model);
        nextModel.currentPlayer = getOpponent(nextModel.currentPlayer);

        MoveList oppMoves;
        getValidMoves(nextModel, oppMoves);

        if (oppMoves.empty()) {
            nextModel.gameOver = true;
            return evaluateBoard(nextModel, maximizingPlayer);
        }

        return minimax(nextModel, depth - 1, !isMaximizing, maximizingPlayer);
    }

    if (isMaximizing) {
        int maxEval = std::numeric_limits<int>::min();

        for (Move_t move : validMoves) {
            GameModel nextModel = copyModel(model);
            playMove(nextModel, move);

            int eval = minimax(nextModel, depth - 1, false, maximizingPlayer);
            maxEval = std::max(maxEval, eval);
        }

        return maxEval;
    }
    else {
        int minEval = std::numeric_limits<int>::max();

        for (Move_t move : validMoves) {
            GameModel nextModel = copyModel(model);
            playMove(nextModel, move);

            int eval = minimax(nextModel, depth - 1, true, maximizingPlayer);
            minEval = std::min(minEval, eval);
        }

        return minEval;
    }
}

Move_t AINormal::getBestMove(GameModel& model) {
    nodesExplored = 0;

    MoveList validMoves;
    getValidMoves(model, validMoves);

    if (validMoves.empty()) {
        return MOVE_NONE;
    }

    PlayerColor_t currentPlayer = getCurrentPlayer(model);
    Move_t bestMove = validMoves[0];
    int bestScore = std::numeric_limits<int>::min();

    std::cout << "[AINormal] Evaluating " << validMoves.size()
        << " moves at depth " << MAX_DEPTH << "..." << std::endl;

    for (Move_t move : validMoves) {
        GameModel nextModel = copyModel(model);
        playMove(nextModel, move);

        int score = minimax(nextModel, MAX_DEPTH - 1, false, currentPlayer);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    std::cout << "[AINormal] Best move: " << (int)bestMove
        << " (score: " << bestScore
        << ", nodes: " << nodesExplored << ")" << std::endl;

    return bestMove;
}