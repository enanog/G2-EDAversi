/**
 * @brief Implements the Reversi game model
 * @author Marc S. Ressl
 * @modifiers:
 *			Agustin Valenzuela,
 *			Alex Petersen,
 *			Dylan Frigerio,
 *			Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "model.h"

#include <cstdlib>  // abs
#include <iostream>
#include "raylib.h" // for GetTime()

// Bitboard edge masks
#define FILE_A 0x0101010101010101ULL
#define FILE_H 0x8080808080808080ULL
#define RANK_1 0x00000000000000FFULL
#define RANK_8 0xFF00000000000000ULL

#define DIRECTION_COUNT 8

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

namespace {

    // Initial discs: black and white starting positions (indices)
    const Move_t initialPosition[2][2] = { {28, 35}, {27, 36} };

    // Direction offsets in bit index space for NW,N,NE,W,E,SW,S,SE
    const int8_t DIRECTIONS[8] = {
        -9,   -8,   -7,
        -1, /* O */  1,
         7,    8,     9
    };

    /**
     * @brief Check if we can advance from pos by step without wrapping edges.
     *
     * Prevents horizontal wrap-around and out-of-bounds steps.
     */
    inline bool canContinueInDirection(Move_t pos, int8_t step) {
        Move_t nextPos = static_cast<Move_t>(pos + step);

        if (nextPos < 0 || nextPos >= 64) {
            return false;
        }

        int currentRow = pos / 8;
        int nextRow = nextPos / 8;

        // Horizontal step that changes row -> wrap-around
        if ((step == -1 || step == 1) && (currentRow != nextRow)) {
            return false;
        }

        // Diagonal steps must move exactly one row
        if ((step == -9 || step == -7 || step == 7 || step == 9) &&
            (std::abs(nextRow - currentRow) != 1)) {
            return false;
        }

        return true;
    }

    /**
     * @brief Accumulate flipped discs in a single direction.
     *
     * Scans from startPos + step and accumulates opponent discs until
     * a friendly disc closes the sequence (then returns the flips).
     * Returns 0 if sequence is not closed or invalid.
     */
    uint64_t getFlipsInDirection(uint64_t player, uint64_t opponent, Move_t startPos, int8_t step, int /*dir*/) {
        if (startPos < 0 || startPos >= 64) {
            return 0ULL;
        }

        if (!canContinueInDirection(startPos, step)) {
            return 0ULL;
        }

        uint64_t flips = 0ULL;
        Move_t curPos = static_cast<Move_t>(startPos + step);
        bool foundOpponent = false;

        while (curPos >= 0 && curPos < 64) {
            uint64_t curBit = 1ULL << curPos;

            if (opponent & curBit) {
                flips |= curBit;
                foundOpponent = true;
            }
            else if (player & curBit) {
                // Only valid if we passed over at least one opponent disc
                return foundOpponent ? flips : 0ULL;
            }
            else {
                // Empty square breaks the sequence
                return 0ULL;
            }

            if (!canContinueInDirection(curPos, step)) {
                break;
            }
            curPos = static_cast<Move_t>(curPos + step);
        }

        return 0ULL;
    }

    // Shift helpers with edge masking (used by Kogge-Stone generation)
    inline uint64_t shiftN(uint64_t bb) { return bb >> 8; }
    inline uint64_t shiftS(uint64_t bb) { return bb << 8; }
    inline uint64_t shiftE(uint64_t bb) { return (bb & ~FILE_H) << 1; }
    inline uint64_t shiftW(uint64_t bb) { return (bb & ~FILE_A) >> 1; }
    inline uint64_t shiftNE(uint64_t bb) { return (bb & ~FILE_H) >> 7; }
    inline uint64_t shiftNW(uint64_t bb) { return (bb & ~FILE_A) >> 9; }
    inline uint64_t shiftSE(uint64_t bb) { return (bb & ~FILE_H) << 9; }
    inline uint64_t shiftSW(uint64_t bb) { return (bb & ~FILE_A) << 7; }

    /**
     * @brief Generate moves in one direction using repeated propagation (Kogge-Stone).
     *
     * This produces candidate move squares by propagating captures and
     * finally intersecting with empty squares.
     */
    uint64_t generateMovesInDirection(uint64_t player, uint64_t opponent, uint64_t(*shiftFunc)(uint64_t)) {
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

// ---------------------------------------------------------------------------
// Core bitboard operations
// ---------------------------------------------------------------------------

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

uint64_t calculateFlips(uint64_t player, uint64_t opponent, Move_t move) {
    if (move < 0 || move >= 64) {
        return 0ULL;
    }

    uint64_t moveBit = 1ULL << move;
    if ((player & moveBit) || (opponent & moveBit)) {
        return 0ULL;
    }

    uint64_t allFlips = 0ULL;
    for (int dir = 0; dir < DIRECTION_COUNT; ++dir) {
        uint64_t flips = getFlipsInDirection(player, opponent, move, DIRECTIONS[dir], dir);
        allFlips |= flips;
    }

    return allFlips;
}

int countBits(uint64_t bitmap) {
#if defined(_MSC_VER)
    return static_cast<int>(__popcnt64(bitmap));
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(bitmap);
#else
    int count = 0;
    while (bitmap) {
        bitmap &= (bitmap - 1);
        ++count;
    }
    return count;
#endif
}

Move_t bitScanForward(uint64_t bb) {
#if defined(_MSC_VER)
    unsigned long idx;
    _BitScanForward64(&idx, bb);
    return static_cast<Move_t>(idx);
#elif defined(__GNUC__) || defined(__clang__)
    return static_cast<Move_t>(__builtin_ctzll(bb));
#else
    Move_t idx = 0;
    while (((bb >> idx) & 1ULL) == 0) ++idx;
    return idx;
#endif
}

// ---------------------------------------------------------------------------
// Game model functions
// ---------------------------------------------------------------------------

void initModel(GameModel& model) {
    // Put model into a neutral default state (no active game)
    model.gameOver = true;
    model.playerTime[0] = 0.0;
    model.playerTime[1] = 0.0;
    model.turnStartTime = 0.0;
    model.board.black = 0ULL;
    model.board.white = 0ULL;
    model.aiThinking = false;
    model.aiMove = MOVE_NONE;
	model.pauseTimers = false;
    model.playedPass = false;
}

void startModel(GameModel& model) {
    // Reset board and timers, set initial pieces and current player
    model.board.black = 0ULL;
    model.board.white = 0ULL;
    model.gameOver = false;
    model.currentPlayer = PLAYER_BLACK;
    model.playerTime[0] = 0.0;
    model.playerTime[1] = 0.0;
    model.turnStartTime = GetTime();
    model.aiThinking = false;
    model.aiMove = MOVE_NONE;

    SET_BIT(model.board.black, initialPosition[0][0]);
    SET_BIT(model.board.black, initialPosition[0][1]);
    SET_BIT(model.board.white, initialPosition[1][0]);
    SET_BIT(model.board.white, initialPosition[1][1]);
}

PlayerColor_t getCurrentPlayer(const GameModel& model) {
    return model.currentPlayer;
}

int getScore(const GameModel& model, PlayerColor_t player) {
    uint64_t bitmap = (player == PLAYER_BLACK) ? model.board.black : model.board.white;
    return countBits(bitmap);
}

double getTimer(const GameModel& model, PlayerColor_t player) {
    // Compute accumulated time plus current turn if applicable.
    double accumulatedTime = model.playerTime[player];

    if (model.pauseTimers) {
        // Do not count time during pass-message display
        return accumulatedTime;
    }

    if (!model.gameOver && (player == model.currentPlayer)) {
        double currentTurnTime = GetTime() - model.turnStartTime;
        return accumulatedTime + currentTurnTime;
    }

    return accumulatedTime;
}

PieceState_t getBoardPiece(const GameModel& model, Move_t move) {
    if (GET_BIT(model.board.black, move)) return STATE_BLACK;
    if (GET_BIT(model.board.white, move)) return STATE_WHITE;
    return STATE_EMPTY;
}

void setBoardPiece(GameModel& model, Move_t move, PieceState_t piece) {
    // Update bitboards consistently for the target square
    if (piece == STATE_BLACK) {
        SET_BIT(model.board.black, move);
        CLEAR_BIT(model.board.white, move);
    }
    else if (piece == STATE_WHITE) {
        SET_BIT(model.board.white, move);
        CLEAR_BIT(model.board.black, move);
    }
    else { // STATE_EMPTY
        CLEAR_BIT(model.board.black, move);
        CLEAR_BIT(model.board.white, move);
    }
}

bool isSquareValid(Move_t pos, int dir) {
    if (pos < 0 || pos >= 64) return false;
    if (dir == NONE) return true;

    uint64_t bit = 1ULL << pos;

    if ((dir == W || dir == NW || dir == SW) && (bit & FILE_A)) return false;
    if ((dir == E || dir == NE || dir == SE) && (bit & FILE_H)) return false;
    if ((dir == N || dir == NW || dir == NE) && (bit & RANK_1)) return false;
    if ((dir == S || dir == SW || dir == SE) && (bit & RANK_8)) return false;

    return true;
}

void getValidMoves(const GameModel& model, MoveList& validMoves) {
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

bool playMove(GameModel& model, Move_t move) {
    // Validate basic move constraints
    if (move < 0 || move >= 64) return false;
    if (model.gameOver) return false;

    PlayerColor_t currentPlayer = getCurrentPlayer(model);
    uint64_t player = getPlayerBitboard(model.board, currentPlayer);
    uint64_t opponent = getOpponentBitboard(model.board, currentPlayer);

    uint64_t moveBit = 1ULL << move;
    if ((player & moveBit) || (opponent & moveBit)) return false;

    // Compute flips; move invalid if no flips
    uint64_t flips = calculateFlips(player, opponent, move);
    if (flips == 0ULL) return false;

    // Apply move and flips atomically
    if (currentPlayer == PLAYER_BLACK) {
        model.board.black |= moveBit | flips;
        model.board.white &= ~flips;
    }
    else {
        model.board.white |= moveBit | flips;
        model.board.black &= ~flips;
    }

    // Update timers and switch player
    double currentTime = GetTime();
    model.playerTime[model.currentPlayer] += (currentTime - model.turnStartTime);

    model.currentPlayer = getOpponent(model.currentPlayer);
    model.turnStartTime = currentTime;

    // If next player has no moves, handle pass or game-over
    MoveList validMoves;
    getValidMoves(model, validMoves);

    if (validMoves.size() == 0) {
        PlayerColor_t playerWhoPassed = model.currentPlayer;

        // Revert to original player to test if both cannot move
        model.currentPlayer = getOpponent(model.currentPlayer);

        MoveList validMoves2;
        getValidMoves(model, validMoves2);

        if (validMoves2.size() == 0) {
            // Both players have no moves -> game over
            model.gameOver = true;
            model.turnStartTime = 0.0;
            return true;
        }

        // Otherwise signal a pass and record time
		model.pauseTimers = true;
		model.playedPass = true;
    }

    return true;
}

// ---------------------------------------------------------------------------
// AI helper functions
// ---------------------------------------------------------------------------

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
    if (hasValidMoves(board, player)) return false;
    PlayerColor_t opponent = getOpponent(player);
    return !hasValidMoves(board, opponent);
}

int getScoreDiff(const Board_t& board, PlayerColor_t player) {
    int playerScore = countBits(getPlayerBitboard(board, player));
    int opponentScore = countBits(getOpponentBitboard(board, player));
    return playerScore - opponentScore;
}

BoardState_t makeMove(Board_t& board, PlayerColor_t& currentPlayer, Move_t move) {
    BoardState_t state;
    state.black = board.black;
    state.white = board.white;
    state.player = currentPlayer;

    if (move < 0 || move >= 64) {
        return state;
    }

    uint64_t player = getPlayerBitboard(board, currentPlayer);
    uint64_t opponent = getOpponentBitboard(board, currentPlayer);
    uint64_t flips = calculateFlips(player, opponent, move);

    if (flips == 0ULL) return state;

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

bool isMoveValid(const Board_t& board, PlayerColor_t player, Move_t move) {
    if (move < 0 || move >= 64) return false;
    if (!isEmpty(board, move)) return false;

    uint64_t playerBB = getPlayerBitboard(board, player);
    uint64_t opponentBB = getOpponentBitboard(board, player);

    uint64_t flips = calculateFlips(playerBB, opponentBB, move);
    return flips != 0ULL;
}