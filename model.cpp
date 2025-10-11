/**
 * @brief Implements the Reversi game model - ROBUST VERSION
 * @author Marc S. Ressl
 * @modified: Bug fixes and robustness improvements
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "model.h"

#include <stdint.h>
#include <bitset>
#include <iostream>

#include "raylib.h"

 // Bitboard masks for board edges
#define FILE_A 0x0101010101010101ULL  // Left column   (x=0)
#define FILE_H 0x8080808080808080ULL  // Right column  (x=7)
#define RANK_1 0x00000000000000FFULL  // Top row       (y=0)
#define RANK_8 0xFF00000000000000ULL  // Bottom row    (y=7)

#define DIRECTION_COUNT 8

// ============================================================================
// Private declarations
// ============================================================================

namespace {

    const Move_t initialPosition[2][2] = { {28, 35}, {27, 36} };

    /**
     * @brief Direction offsets for bit index movement
     */
    const int8_t DIRECTIONS[8] = {
        -9,   -8,   -7, // NW, N, NE
        -1, /* O */  1, // W,     E
         7,    8,    9  // SW, S, SE
    };

    /**
     * @brief Validates if we can continue in a direction from current position
     *
     * CAMBIO 1: Nueva función para validar movimientos direccionales
     * JUSTIFICACIÓN: Previene wrap-around en bordes del tablero
     */
    inline bool canContinueInDirection(Move_t pos, int8_t step) {
        Move_t nextPos = pos + step;

        // Fuera de rango del tablero
        if (nextPos < 0 || nextPos >= 64)
            return false;

        // Detectar wrap-around horizontal
        int currentRow = pos / 8;
        int nextRow = nextPos / 8;

        // Si step es horizontal (-1, +1) pero cambia de fila = wrap-around
        if (step == -1 || step == 1) {
            if (currentRow != nextRow)
                return false;
        }

        // Si step incluye componente horizontal, verificar wrap-around
        if (step == -9 || step == -7 || step == 7 || step == 9) {
            // Diferencia de filas debe ser exactamente 1
            int rowDiff = abs(nextRow - currentRow);
            if (rowDiff != 1)
                return false;
        }

        return true;
    }

    /**
     * @brief Calculates the discs that would be flipped in one direction.
     *
     * CAMBIO 2: Validación robusta de límites y wrap-around
     * JUSTIFICACIÓN: Previene bugs cuando se colocan fichas en bordes
     */
    uint64_t getFlipsInDirection(
        uint64_t player, uint64_t opponent, Move_t startPos, int8_t step, int dir) {

        // CAMBIO 2.1: Validar que startPos es válida
        if (startPos < 0 || startPos >= 64)
            return 0ULL;

        // CAMBIO 2.2: Validar que podemos movernos en esta dirección
        if (!canContinueInDirection(startPos, step))
            return 0ULL;

        uint64_t flips = 0ULL;
        Move_t curPos = startPos + step;
        bool foundOpponent = false;

        // CAMBIO 2.3: Acumulamos fichas oponentes hasta encontrar ficha propia
        while (curPos >= 0 && curPos < 64) {
            uint64_t curBit = 1ULL << curPos;

            if (opponent & curBit) {
                // Encontramos ficha oponente - marcar para voltear
                flips |= curBit;
                foundOpponent = true;
            }
            else if (player & curBit) {
                // Encontramos ficha propia - validar secuencia
                // CAMBIO 2.4: Solo retornar flips si hubo al menos UN oponente
                if (foundOpponent) {
                    return flips;
                }
                return 0ULL;
            }
            else {
                // Casilla vacía - secuencia interrumpida
                return 0ULL;
            }

            // CAMBIO 2.5: Validar próximo paso antes de avanzar
            if (!canContinueInDirection(curPos, step))
                break;

            curPos += step;
        }

        // No se cerró la secuencia con ficha propia
        return 0ULL;
    }

    /**
     * @brief Shift operations with edge masking
     * SIN CAMBIOS - Estas funciones ya están correctas
     */
    inline uint64_t shiftN(uint64_t bb) {
        return bb >> 8;
    }
    inline uint64_t shiftS(uint64_t bb) {
        return bb << 8;
    }
    inline uint64_t shiftE(uint64_t bb) {
        return (bb & ~FILE_H) << 1;
    }
    inline uint64_t shiftW(uint64_t bb) {
        return (bb & ~FILE_A) >> 1;
    }
    inline uint64_t shiftNE(uint64_t bb) {
        return (bb & ~FILE_H) >> 7;
    }
    inline uint64_t shiftNW(uint64_t bb) {
        return (bb & ~FILE_A) >> 9;
    }
    inline uint64_t shiftSE(uint64_t bb) {
        return (bb & ~FILE_H) << 9;
    }
    inline uint64_t shiftSW(uint64_t bb) {
        return (bb & ~FILE_A) << 7;
    }

    /**
     * @brief Generates valid moves in one direction using kogge-stone propagation
     * SIN CAMBIOS - El algoritmo Kogge-Stone está correcto
     */
    uint64_t generateMovesInDirection(uint64_t player,
        uint64_t opponent,
        uint64_t(*shiftFunc)(uint64_t)) {
        uint64_t empty = ~(player | opponent);

        uint64_t candidates = shiftFunc(player) & opponent;

        candidates |= shiftFunc(candidates) & opponent;
        candidates |= shiftFunc(candidates) & opponent;
        candidates |= shiftFunc(candidates) & opponent;
        candidates |= shiftFunc(candidates) & opponent;
        candidates |= shiftFunc(candidates) & opponent;

        return shiftFunc(candidates) & empty;
    }
}

// ============================================================================
// Core bitboard operations
// ============================================================================

uint64_t getValidMovesBitmap(uint64_t player, uint64_t opponent) {
    uint64_t legal = 0ULL;

    legal |= generateMovesInDirection(player, opponent, shiftN);
    legal |= generateMovesInDirection(player, opponent, shiftS);
    legal |= generateMovesInDirection(player, opponent, shiftE);
    legal |= generateMovesInDirection(player, opponent, shiftW);
    legal |= generateMovesInDirection(player, opponent, shiftNE);
    legal |= generateMovesInDirection(player, opponent, shiftNW);
    legal |= generateMovesInDirection(player, opponent, shiftSE);
    legal |= generateMovesInDirection(player, opponent, shiftSW);

    return legal;
}

/**
 * CAMBIO 3: Validación adicional en calculateFlips
 * JUSTIFICACIÓN: Verificar que el movimiento es válido antes de calcular
 */
uint64_t calculateFlips(uint64_t player, uint64_t opponent, Move_t move) {
    // CAMBIO 3.1: Validar que el movimiento está en rango
    if (move < 0 || move >= 64)
        return 0ULL;

    // CAMBIO 3.2: Validar que la casilla está vacía
    uint64_t moveBit = 1ULL << move;
    if ((player & moveBit) || (opponent & moveBit))
        return 0ULL;

    uint64_t allFlips = 0ULL;

    for (int dir = 0; dir < DIRECTION_COUNT; dir++) {
        uint64_t flips = getFlipsInDirection(player, opponent, move, DIRECTIONS[dir], dir);
        allFlips |= flips;
    }

    return allFlips;
}

int countBits(uint64_t bitmap) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(bitmap);
#elif defined(_MSC_VER)
    return __popcnt64(bitmap);
#else
    // Fallback: Brian Kernighan
    int count = 0;
    while (bitmap) {
        bitmap &= (bitmap - 1);
        count++;
    }
    return count;
#endif
}

Move_t bitScanForward(uint64_t bb) {
#if defined(_MSC_VER)
    unsigned long idx;
    _BitScanForward64(&idx, bb);
    return (Move_t)idx;
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_ctzll(bb);
#else
    Move_t idx = 0;
    while (((bb >> idx) & 1ULL) == 0)
        idx++;
    return idx;
#endif
}

// ============================================================================
// Game model functions
// ============================================================================

void initModel(GameModel& model) {
    model.gameOver = true;
    model.playerTime[0] = 0;
    model.playerTime[1] = 0;
    model.turnStartTime = 0;
    model.board.black = 0;
    model.board.white = 0;
    model.aiThinking = false;
    model.aiMove = MOVE_NONE;
    model.showPassMessage = false;
    model.passedPlayer = PLAYER_BLACK;
    model.passMessageStartTime = 0;
}

void startModel(GameModel& model) {
    model.board.black = 0;
    model.board.white = 0;
    model.gameOver = false;
    model.currentPlayer = PLAYER_BLACK;
    model.playerTime[0] = 0;
    model.playerTime[1] = 0;
    model.turnStartTime = GetTime();
    model.aiThinking = false;
    model.aiMove = MOVE_NONE;

    SET_BIT(model.board.black, initialPosition[0][0]);
    SET_BIT(model.board.black, initialPosition[0][1]);
    SET_BIT(model.board.white, initialPosition[1][0]);
    SET_BIT(model.board.white, initialPosition[1][1]);
}

PlayerColor_t getCurrentPlayer(GameModel& model) {
    return model.currentPlayer;
}

int getScore(GameModel& model, PlayerColor_t player) {
    uint64_t bitmap = (player == PLAYER_BLACK) ? model.board.black : model.board.white;
    return countBits(bitmap);
}

double getTimer(GameModel& model, PlayerColor_t player) {
    double accumulatedTime = model.playerTime[player];

    if (model.showPassMessage) {
        return accumulatedTime;  // No contar tiempo
    }

    if (!model.gameOver && (player == model.currentPlayer)) {
        double currentTurnTime = GetTime() - model.turnStartTime;
        return accumulatedTime + currentTurnTime;
    }

    return accumulatedTime;
}

PieceState_t getBoardPiece(GameModel& model, Move_t move) {
    if (GET_BIT(model.board.black, move))
        return STATE_BLACK;
    if (GET_BIT(model.board.white, move))
        return STATE_WHITE;
    return STATE_EMPTY;
}

void setBoardPiece(GameModel& model, Move_t move, PieceState_t piece) {
    if (piece == STATE_BLACK) {
        SET_BIT(model.board.black, move);
        CLEAR_BIT(model.board.white, move);
    }
    else if (piece == STATE_WHITE) {
        SET_BIT(model.board.white, move);
        CLEAR_BIT(model.board.black, move);
    }
    else {  // STATE_EMPTY
        CLEAR_BIT(model.board.black, move);
        CLEAR_BIT(model.board.white, move);
    }
}

bool isSquareValid(Move_t pos, int dir) {
    if (pos < 0 || pos >= 64)
        return false;

    if (dir == NONE)
        return true;

    uint64_t bit = 1ULL << pos;

    if ((dir == W || dir == NW || dir == SW) && (bit & FILE_A))
        return false;
    if ((dir == E || dir == NE || dir == SE) && (bit & FILE_H))
        return false;
    if ((dir == N || dir == NW || dir == NE) && (bit & RANK_1))
        return false;
    if ((dir == S || dir == SW || dir == SE) && (bit & RANK_8))
        return false;

    return true;
}

void getValidMoves(GameModel& model, MoveList& validMoves) {
    validMoves.clear();

    uint64_t player = getPlayerBitboard(model.board, getCurrentPlayer(model));
    uint64_t opponent = getOpponentBitboard(model.board, getCurrentPlayer(model));
    uint64_t validMovesBitmap = getValidMovesBitmap(player, opponent);

    while (validMovesBitmap) {
        Move_t move = bitScanForward(validMovesBitmap);
        validMoves.push_back(move);
        validMovesBitmap &= validMovesBitmap - 1;
    }
}

/**
 * CAMBIO 4: Validación robusta en playMove
 * JUSTIFICACIÓN: Asegurar que solo se ejecuten movimientos válidos
 */
bool playMove(GameModel& model, Move_t move) {
    // CAMBIO 4.1: Validar rango del movimiento
    if (move < 0 || move >= 64)
        return false;

    // CAMBIO 4.2: Validar que no estamos en game over
    if (model.gameOver)
        return false;

    PlayerColor_t currentPlayer = getCurrentPlayer(model);
    uint64_t player = getPlayerBitboard(model.board, currentPlayer);
    uint64_t opponent = getOpponentBitboard(model.board, currentPlayer);

    // CAMBIO 4.3: Validar que la casilla está vacía
    uint64_t moveBit = 1ULL << move;
    if ((player & moveBit) || (opponent & moveBit))
        return false;

    // CAMBIO 4.4: Calcular flips - si es 0, el movimiento es inválido
    uint64_t flips = calculateFlips(player, opponent, move);
    if (flips == 0ULL)
        return false;

    // Aplicar movimiento y flips
    if (currentPlayer == PLAYER_BLACK) {
        model.board.black |= moveBit | flips;
        model.board.white &= ~flips;
    }
    else {
        model.board.white |= moveBit | flips;
        model.board.black &= ~flips;
    }

    // Actualizar timer
    double currentTime = GetTime();
    model.playerTime[model.currentPlayer] += currentTime - model.turnStartTime;

    // Cambiar jugador
    model.currentPlayer = getOpponent(model.currentPlayer);
    model.turnStartTime = currentTime;

    // Verificar si el siguiente jugador tiene movimientos válidos
    MoveList validMoves;
    getValidMoves(model, validMoves);

    if (validMoves.size() == 0) {
        PlayerColor_t playerWhoPassed = model.currentPlayer;
        // Volver al jugador original
        model.currentPlayer = getOpponent(model.currentPlayer);

        MoveList validMoves2;
        getValidMoves(model, validMoves2);

        if (validMoves2.size() == 0)
        {
            model.gameOver = true;
			model.turnStartTime = 0;
            std::cout << "[Model] Game over detected after both players passed." << std::endl;
			return true;
        }
        
        model.showPassMessage = true;
        model.passedPlayer = playerWhoPassed;
        model.passMessageStartTime = GetTime();
    }

    return true;
}

// ============================================================================
// AI helper functions - CON MEJORAS
// ============================================================================

void getValidMovesAI(const Board_t& board, PlayerColor_t player, MoveList& moves) {
    moves.clear();

    uint64_t playerBB = getPlayerBitboard(board, player);
    uint64_t opponentBB = getOpponentBitboard(board, player);
    uint64_t validMovesBitmap = getValidMovesBitmap(playerBB, opponentBB);

    while (validMovesBitmap) {
        Move_t move = bitScanForward(validMovesBitmap);
        moves.push_back(move);
        validMovesBitmap &= validMovesBitmap - 1;
    }
}

bool hasValidMoves(const Board_t& board, PlayerColor_t player) {
    uint64_t playerBB = getPlayerBitboard(board, player);
    uint64_t opponentBB = getOpponentBitboard(board, player);
    return getValidMovesBitmap(playerBB, opponentBB) != 0ULL;
}

bool isTerminal(const Board_t& board, PlayerColor_t player) {
    if (hasValidMoves(board, player))
        return false;

    PlayerColor_t opponent = getOpponent(player);
    return !hasValidMoves(board, opponent);
}

int getScoreDiff(const Board_t& board, PlayerColor_t player) {
    int playerScore = countBits(getPlayerBitboard(board, player));
    int opponentScore = countBits(getOpponentBitboard(board, player));
    return playerScore - opponentScore;
}

/**
 * CAMBIO 5: Validación en makeMove para AI
 * JUSTIFICACIÓN: Prevenir estados inválidos durante búsqueda
 */
BoardState_t makeMove(Board_t& board, PlayerColor_t& currentPlayer, Move_t move) {
    // Guardar estado
    BoardState_t state;
    state.black = board.black;
    state.white = board.white;
    state.player = currentPlayer;

    // CAMBIO 5.1: Validación defensiva
    if (move < 0 || move >= 64)
        return state;

    uint64_t player = getPlayerBitboard(board, currentPlayer);
    uint64_t opponent = getOpponentBitboard(board, currentPlayer);
    uint64_t flips = calculateFlips(player, opponent, move);

    // CAMBIO 5.2: Si no hay flips, retornar sin cambios
    if (flips == 0ULL)
        return state;

    // Aplicar movimiento
    uint64_t moveBit = 1ULL << move;
    if (currentPlayer == PLAYER_BLACK) {
        board.black |= moveBit | flips;
        board.white &= ~flips;
    }
    else {
        board.white |= moveBit | flips;
        board.black &= ~flips;
    }

    currentPlayer = getOpponent(currentPlayer);

    return state;
}

void unmakeMove(Board_t& board, PlayerColor_t& currentPlayer, const BoardState_t& state) {
    board.black = state.black;
    board.white = state.white;
    currentPlayer = state.player;
}

int getMoveCount(const Board_t& board, PlayerColor_t player) {
    uint64_t playerBB = getPlayerBitboard(board, player);
    uint64_t opponentBB = getOpponentBitboard(board, player);
    return countBits(getValidMovesBitmap(playerBB, opponentBB));
}

/**
 * CAMBIO 6: isMoveValid con doble verificación
 * JUSTIFICACIÓN: Asegurar consistencia entre algoritmos
 */
bool isMoveValid(const Board_t& board, PlayerColor_t player, Move_t move) {
    if (move < 0 || move >= 64)
        return false;

    // CAMBIO 6.1: Verificar que la casilla está vacía
    if (!isEmpty(board, move))
        return false;

    uint64_t playerBB = getPlayerBitboard(board, player);
    uint64_t opponentBB = getOpponentBitboard(board, player);

    // CAMBIO 6.2: Verificar que voltea al menos una ficha
    uint64_t flips = calculateFlips(playerBB, opponentBB, move);
    return flips != 0ULL;
}