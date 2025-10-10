/**
 * @brief Implements the Reversi game model
 * @author Marc S. Ressl
 * @modified: Refactored to use unified Move_t representation
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

    /* Initial four pieces setup in the board center
     *
     *     A B C D E F G H          A  B  C  D  E  F  G  H
     * 1 | . . . . . . . . |    1 | 0  1  2  3  4  5  6  7|
     * 2 | . . . . . . . . |    2 | 8  9 10 11 12 13 14 15|
     * 3 | . . . . . . . . |    3 |16 17 18 19 20 21 22 23|
     * 4 | . . . W B . . . |    4 |24 25 26 27 28 29 30 31|
     * 5 | . . . B W . . . |    5 |32 33 34 35 36 37 38 39|
     * 6 | . . . . . . . . |    6 |40 41 42 43 44 45 46 47|
     * 7 | . . . . . . . . |    7 |48 49 50 51 52 53 54 55|
     * 8 | . . . . . . . . |    8 |56 57 58 59 60 61 62 63|
     */
    const Move_t initialPosition[2][2] = { {28, 35}, {27, 36} };  // {black}, {white}

    /**
     * @brief Direction offsets for bit index movement
     *
     *    NW  N  NE
     *    ↖   ↑   ↗
     *  W ←   •   → E
     *    ↙   ↓   ↘
     *    SW  S  SE
     */
    const int8_t DIRECTIONS[8] = {
        -9,   -8,   -7, // NW, N, NE
        -1, /* O */  1, // W,     E
         7,    8,    9  // SW, S, SE
    };

    /**
     * @brief Calculates the discs that would be flipped in one direction.
     *
     * @param player Bitboard of current player.
     * @param opponent Bitboard of opponent.
     * @param startPos Starting empty square index.
     * @param step Direction step offset.
     * @param dir Direction index.
     * @return Bitboard with flipped discs, or 0 if none.
     */
    uint64_t getFlipsInDirection(
        uint64_t player, uint64_t opponent, Move_t startPos, int step, int dir) {
        uint64_t flips = 0;
        int curPos = startPos + step;

        while (isSquareValid(curPos, dir)) {
            uint64_t curBit = 1ULL << curPos;

            if (opponent & curBit) {
                flips |= curBit;
            }
            else if (player & curBit) {
                return flips;
            }
            else {
                break;
            }

            curPos += step;
        }

        return 0ULL;
    }

    /**
     * @brief Shift operations with edge masking
     * These ensure pieces don't "wrap around" board edges
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
     *
     * @param player Current player's pieces
     * @param opponent Opponent's pieces
     * @param shiftFunc Function pointer for directional shift
     * @return Bitboard of valid moves in this direction
     */
    uint64_t generateMovesInDirection(uint64_t player,
        uint64_t opponent,
        uint64_t(*shiftFunc)(uint64_t)) {
        uint64_t empty = ~(player | opponent);

        // Start with opponent pieces adjacent to player pieces
        uint64_t candidates = shiftFunc(player) & opponent;

        // Propagate through opponent pieces (kogge-stone algorithm)
        // Each iteration doubles the propagation distance
        candidates |= shiftFunc(candidates) & opponent;
        candidates |= shiftFunc(candidates) & opponent;
        candidates |= shiftFunc(candidates) & opponent;
        candidates |= shiftFunc(candidates) & opponent;
        candidates |= shiftFunc(candidates) & opponent;
        // 6 iterations = max propagation of 64 squares (2^6)

        // One final shift into empty squares gives us valid moves
        return shiftFunc(candidates) & empty;
    }
}

// ============================================================================
// Core bitboard operations
// ============================================================================

uint64_t getValidMovesBitmap(uint64_t player, uint64_t opponent) {
    uint64_t legal = 0ULL;

    // Check all 8 directions
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

uint64_t calculateFlips(uint64_t player, uint64_t opponent, Move_t move) {
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
#if defined(_MSC_VER)  // MSVC (Windows)
    unsigned long idx;
    _BitScanForward64(&idx, bb);
    return (Move_t)idx;
#elif defined(__GNUC__) || defined(__clang__)  // GCC/Clang (Linux/macOS)
    return __builtin_ctzll(bb);
#else
    // Fallback
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
        validMovesBitmap &= validMovesBitmap - 1;  // Clear LSB
    }
}

bool playMove(GameModel& model, Move_t move) {
    // Get board state BEFORE placing piece
    PlayerColor_t currentPlayer = getCurrentPlayer(model);
    uint64_t player = getPlayerBitboard(model.board, currentPlayer);
    uint64_t opponent = getOpponentBitboard(model.board, currentPlayer);

    uint64_t flips = calculateFlips(player, opponent, move);

    // Apply move and flips
    uint64_t moveBit = 1ULL << move;
    if (currentPlayer == PLAYER_BLACK) {
        model.board.black |= moveBit | flips;
        model.board.white &= ~flips;
    }
    else {
        model.board.white |= moveBit | flips;
        model.board.black &= ~flips;
    }

    // Update timer
    double currentTime = GetTime();
    model.playerTime[model.currentPlayer] += currentTime - model.turnStartTime;

    // Swap player
    model.currentPlayer = getOpponent(model.currentPlayer);
    model.turnStartTime = currentTime;

    // Check if next player has valid moves
    MoveList validMoves;
    getValidMoves(model, validMoves);

    if (validMoves.size() == 0) {
        // Swap player back
        model.currentPlayer = getOpponent(model.currentPlayer);

        MoveList validMoves2;
        getValidMoves(model, validMoves2);

        if (validMoves2.size() == 0)
            model.gameOver = true;
    }

    return true;
}

// ============================================================================
// AI helper functions
// ============================================================================

void getValidMovesAI(const Board_t& board, PlayerColor_t player, MoveList& moves) {
    moves.clear();

    uint64_t playerBB = getPlayerBitboard(board, player);
    uint64_t opponentBB = getOpponentBitboard(board, player);
    uint64_t validMovesBitmap = getValidMovesBitmap(playerBB, opponentBB);

    while (validMovesBitmap) {
        Move_t move = bitScanForward(validMovesBitmap);
        moves.push_back(move);
        validMovesBitmap &= validMovesBitmap - 1;  // Clear LSB
    }
}

bool hasValidMoves(const Board_t& board, PlayerColor_t player) {
    uint64_t playerBB = getPlayerBitboard(board, player);
    uint64_t opponentBB = getOpponentBitboard(board, player);
    return getValidMovesBitmap(playerBB, opponentBB) != 0ULL;
}

bool isTerminal(const Board_t& board, PlayerColor_t player) {
    // Terminal if current player can't move AND opponent can't move
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

BoardState_t makeMove(Board_t& board, PlayerColor_t& currentPlayer, Move_t move) {
    // Save current state
    BoardState_t state;
    state.black = board.black;
    state.white = board.white;
    state.player = currentPlayer;

    // Calculate flips
    uint64_t player = getPlayerBitboard(board, currentPlayer);
    uint64_t opponent = getOpponentBitboard(board, currentPlayer);
    uint64_t flips = calculateFlips(player, opponent, move);

    // Apply move and flips
    uint64_t moveBit = 1ULL << move;
    if (currentPlayer == PLAYER_BLACK) {
        board.black |= moveBit | flips;
        board.white &= ~flips;
    }
    else {
        board.white |= moveBit | flips;
        board.black &= ~flips;
    }

    // Swap player
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

bool isMoveValid(const Board_t& board, PlayerColor_t player, Move_t move) {
    if (move < 0 || move >= 64)
        return false;

    // Square must be empty
    if (!isEmpty(board, move))
        return false;

    uint64_t playerBB = getPlayerBitboard(board, player);
    uint64_t opponentBB = getOpponentBitboard(board, player);

    // Check if this move would flip any pieces
    return calculateFlips(playerBB, opponentBB, move) != 0ULL;
}