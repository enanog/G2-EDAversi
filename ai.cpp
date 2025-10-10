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

#include <cstdlib>
#include <limits>
#include <algorithm>

#include "view.h"
#include "controller.h"

 /*
  Difficulties:
  EASY: 
  NORMAL: minimax with depth n
  HARD: minimax depth n with alpha-beta pruning and improved evaluation function
  EXTREME: in progress
  */

#define HARD_DIFFICULTY

#ifdef HARD_DIFFICULTY
namespace {
    // Adjustable configuration
    const int MAX_DEPTH = 8;
    const int MAX_NODES = 500000;

    int nodesExplored = 0;

    // Positional value table (weight of each square)
    const int POSITION_WEIGHTS[64] = {
        500, -150,  30,  10,  10,  30, -150,  500,
       -150, -250,   0,   0,   0,   0, -250, -150,
         30,    0,   1,   2,   2,   1,    0,   30,
         10,    0,   2,  16,  16,   2,    0,   10,
         10,    0,   2,  16,  16,   2,    0,   10,
         30,    0,   1,   2,   2,   1,    0,   30,
       -150, -250,   0,   0,   0,   0, -250, -150,
        500, -150,  30,  10,  10,  30, -150,  500
    };

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

            if (myScore > oppScore) return 100000;
            if (myScore < oppScore) return -100000;
            return 0;
        }

        int score = 0;
        uint64_t myBoard = getPlayerBitboard(model.board, maximizingPlayer);
        uint64_t oppBoard = getOpponentBitboard(model.board, maximizingPlayer);

        // 1. Positional weights
        for (int i = 0; i < 64; i++) {
            if (GET_BIT(myBoard, i)) score += POSITION_WEIGHTS[i];
            if (GET_BIT(oppBoard, i)) score -= POSITION_WEIGHTS[i];
        }

        // 2. Mobility
        int myMobility = getMoveCount(model.board, maximizingPlayer);
        int oppMobility = getMoveCount(model.board, getOpponent(maximizingPlayer));

        int totalPieces = countBits(myBoard | oppBoard);

        // Mobility is more important in the midgame
        if (totalPieces < 50) {
            score += (myMobility - oppMobility) * 80;
        }
        else {
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
    int alphaBeta(GameModel& model, int depth, int alpha, int beta,
        bool isMaximizing, PlayerColor_t maximizingPlayer) {
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

            return alphaBeta(nextModel, depth - 1, alpha, beta,
                !isMaximizing, maximizingPlayer);
        }

        // Move ordering
        std::sort(validMoves.begin(), validMoves.end(),
            [](Move_t a, Move_t b) {
                return POSITION_WEIGHTS[a] > POSITION_WEIGHTS[b];
            });

        if (isMaximizing) {
            int maxEval = std::numeric_limits<int>::min();

            for (Move_t move : validMoves) {
                GameModel nextModel = copyModel(model);
                playMove(nextModel, move);

                int eval = alphaBeta(nextModel, depth - 1, alpha, beta,
                    false, maximizingPlayer);
                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);

                if (beta <= alpha) {
                    break; // Beta cutoff
                }
            }

            return maxEval;
        }
        else {
            int minEval = std::numeric_limits<int>::max();

            for (Move_t move : validMoves) {
                GameModel nextModel = copyModel(model);
                playMove(nextModel, move);

                int eval = alphaBeta(nextModel, depth - 1, alpha, beta,
                    true, maximizingPlayer);
                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);

                if (beta <= alpha) {
                    break; // Alpha cutoff
                }
            }

            return minEval;
        }
    }
}

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
    }
    else if (totalPieces < 20) {
        // Opening: shallower search
        searchDepth = MAX_DEPTH - 2;
    }

    PlayerColor_t currentPlayer = getCurrentPlayer(model);
    Move_t bestMove = validMoves[0];
    int bestScore = std::numeric_limits<int>::min();

    for (Move_t move : validMoves) {
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

    return bestMove;
}
#endif
