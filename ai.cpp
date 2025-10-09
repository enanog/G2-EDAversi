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
#include <vector>

#include "ai.h"
#include "view.h"
#include "controller.h"

 /*
 Difficulties
 EASY: random
 NORMAL: minimax depth n (FALTA VER QUE PROFUNDIDAD ANALIZA BIEN)
 HARD: minimax depth n with alpha-beta pruning and improved evaluation function
 EXTREME: enzo (en tramite)
 */

#define HARD_DIFFICULTY

#ifdef HARD_DIFFICULTY
namespace {
    // Configuración ajustable
    const int MAX_DEPTH = 8;
    const int MAX_NODES = 500000;

    int nodesExplored = 0;

    // Tabla de valores posicionales (peso de cada casilla)
    const int POSITION_WEIGHTS[64] = {
        500, -150,  30,  10,  10,  30, -150, 500,
       -150, -250,   0,   0,   0,   0, -250, -150,
         30,    0,   1,   2,   2,   1,    0,  30,
         10,    0,   2,  16,  16,   2,    0,  10,
         10,    0,   2,  16,  16,   2,    0,  10,
         30,    0,   1,   2,   2,   1,    0,  30,
       -150, -250,   0,   0,   0,   0, -250, -150,
        500, -150,  30,  10,  10,  30, -150, 500
    };

    /**
     * @brief Cuenta bits activados (popcount) de forma multiplataforma
     */
    inline int popcount64(uint64_t x) {
#if defined(_MSC_VER)
        // Windows con MSVC
        return (int)__popcnt64(x);
#elif defined(__GNUC__) || defined(__clang__)
        // GCC/Clang (Linux/macOS)
        return __builtin_popcountll(x);
#else
        // Fallback portable (algoritmo de Brian Kernighan)
        int count = 0;
        while (x) {
            x &= (x - 1);
            count++;
        }
        return count;
#endif
    }

    /**
     * @brief Encuentra el bit menos significativo (bitscan forward) de forma multiplataforma
     */
    inline int bitScanForward64(uint64_t x) {
#if defined(_MSC_VER)
        // Windows con MSVC
        unsigned long idx;
        _BitScanForward64(&idx, x);
        return (int)idx;
#elif defined(__GNUC__) || defined(__clang__)
        // GCC/Clang
        return __builtin_ctzll(x);
#else
        // Fallback portable
        int idx = 0;
        while (((x >> idx) & 1ULL) == 0) {
            idx++;
        }
        return idx;
#endif
    }

    /**
     * @brief Copia el estado del juego para simulación
     */
    GameModel copyModel(const GameModel& model) {
        GameModel copy;
        copy.board.black = model.board.black;
        copy.board.white = model.board.white;
        copy.currentPlayer = model.currentPlayer;
        copy.gameOver = model.gameOver;
        copy.humanPlayer = model.humanPlayer;
        copy.playerTime[0] = model.playerTime[0];
        copy.playerTime[1] = model.playerTime[1];
        copy.turnTimer = model.turnTimer;
        return copy;
    }

    /**
     * @brief Función de evaluación mejorada con pesos posicionales
     */
    int evaluateBoard(GameModel& model, PlayerColor_t maximizingPlayer) {
        if (model.gameOver) {
            int myScore = getScore(model, maximizingPlayer);
            int oppScore = getScore(model, maximizingPlayer == PLAYER_BLACK ?
                PLAYER_WHITE : PLAYER_BLACK);

            if (myScore > oppScore) return 100000;
            if (myScore < oppScore) return -100000;
            return 0;
        }

        int score = 0;
        uint64_t myBoard = (maximizingPlayer == PLAYER_BLACK) ?
            model.board.black : model.board.white;
        uint64_t oppBoard = (maximizingPlayer == PLAYER_BLACK) ?
            model.board.white : model.board.black;

        // 1. Pesos posicionales
        for (int i = 0; i < 64; i++) {
            if (GET_BIT(myBoard, i)) score += POSITION_WEIGHTS[i];
            if (GET_BIT(oppBoard, i)) score -= POSITION_WEIGHTS[i];
        }

        // 2. Movilidad (muy importante)
        Moves myMoves, oppMoves;
        getValidMoves(model, myMoves);

        model.currentPlayer = (model.currentPlayer == PLAYER_BLACK) ?
            PLAYER_WHITE : PLAYER_BLACK;
        getValidMoves(model, oppMoves);
        model.currentPlayer = (model.currentPlayer == PLAYER_BLACK) ?
            PLAYER_WHITE : PLAYER_BLACK;

        int totalPieces = popcount64(myBoard | oppBoard);

        // Movilidad más importante en medio juego
        if (totalPieces < 50) {
            score += ((int)myMoves.size() - (int)oppMoves.size()) * 80;
        }
        else {
            score += ((int)myMoves.size() - (int)oppMoves.size()) * 30;
        }

        // 3. Paridad de fichas (importante en late game)
        if (totalPieces > 50) {
            int myPieces = popcount64(myBoard);
            int oppPieces = popcount64(oppBoard);
            score += (myPieces - oppPieces) * 150;
        }

        // 4. Estabilidad (fichas en esquinas que no pueden ser volteadas)
        const uint64_t corners = 0x8100000000000081ULL;
        score += popcount64(myBoard & corners) * 500;
        score -= popcount64(oppBoard & corners) * 500;

        return score;
    }

    /**
     * @brief Minimax con poda Alpha-Beta
     */
    int alphaBeta(GameModel& model, int depth, int alpha, int beta,
        bool isMaximizing, PlayerColor_t maximizingPlayer) {
        nodesExplored++;

        if (depth == 0 || model.gameOver || nodesExplored >= MAX_NODES) {
            return evaluateBoard(model, maximizingPlayer);
        }

        Moves validMoves;
        getValidMoves(model, validMoves);

        if (validMoves.empty()) {
            GameModel nextModel = copyModel(model);
            nextModel.currentPlayer = (nextModel.currentPlayer == PLAYER_BLACK) ?
                PLAYER_WHITE : PLAYER_BLACK;

            Moves oppMoves;
            getValidMoves(nextModel, oppMoves);
            if (oppMoves.empty()) {
                nextModel.gameOver = true;
                return evaluateBoard(nextModel, maximizingPlayer);
            }

            return alphaBeta(nextModel, depth - 1, alpha, beta,
                !isMaximizing, maximizingPlayer);
        }

        // Ordenamiento de movimientos (optimización)
        // Evaluar esquinas primero para mejor poda
        std::sort(validMoves.begin(), validMoves.end(),
            [](const Square_t& a, const Square_t& b) {
                int idxA = GET_SQUARE_BIT_INDEX(a.x, a.y);
                int idxB = GET_SQUARE_BIT_INDEX(b.x, b.y);
                return POSITION_WEIGHTS[idxA] > POSITION_WEIGHTS[idxB];
            });

        if (isMaximizing) {
            int maxEval = std::numeric_limits<int>::min();

            for (const Square_t& move : validMoves) {
                GameModel nextModel = copyModel(model);
                int8_t n = GET_SQUARE_BIT_INDEX(move.x, move.y);
                playMove(nextModel, n);

                int eval = alphaBeta(nextModel, depth - 1, alpha, beta,
                    false, maximizingPlayer);
                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);

                if (beta <= alpha) {
                    break; // Poda Beta
                }
            }

            return maxEval;
        }
        else {
            int minEval = std::numeric_limits<int>::max();

            for (const Square_t& move : validMoves) {
                GameModel nextModel = copyModel(model);
                int8_t n = GET_SQUARE_BIT_INDEX(move.x, move.y);
                playMove(nextModel, n);

                int eval = alphaBeta(nextModel, depth - 1, alpha, beta,
                    true, maximizingPlayer);
                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);

                if (beta <= alpha) {
                    break; // Poda Alpha
                }
            }

            return minEval;
        }
    }
}

Square_t getBestMove(GameModel& model)
{
    nodesExplored = 0;

    Moves validMoves;
    getValidMoves(model, validMoves);

    if (validMoves.empty()) {
        return GAME_INVALID_SQUARE;
    }

    // Ajuste dinámico de profundidad según fase del juego
    int totalPieces = getScore(model, PLAYER_BLACK) + getScore(model, PLAYER_WHITE);
    int searchDepth = MAX_DEPTH;

    if (totalPieces > 52) {
        // Endgame: búsqueda completa si es posible
        searchDepth = std::min(64 - totalPieces, 15);
    }
    else if (totalPieces < 20) {
        // Early game: menos profundidad
        searchDepth = MAX_DEPTH - 2;
    }

    PlayerColor_t currentPlayer = getCurrentPlayer(model);
    Square_t bestMove = validMoves[0];
    int bestScore = std::numeric_limits<int>::min();

    for (const Square_t& move : validMoves) {
        GameModel nextModel = copyModel(model);
        int8_t n = GET_SQUARE_BIT_INDEX(move.x, move.y);
        playMove(nextModel, n);

        int score = alphaBeta(nextModel, searchDepth - 1,
            std::numeric_limits<int>::min(),
            std::numeric_limits<int>::max(),
            false, currentPlayer);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        // Mantener UI responsiva cada 1000 nodos
        if (nodesExplored % 1000 == 0) {
            drawView(model);
        }
    }

    return bestMove;
}
#endif