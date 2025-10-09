// /**
//  * @brief Implements the Reversi game model
//  * @author Marc S. Ressl
//  * @modified:
//  *			Agustin Valenzuela,
//  *			Alex Petersen,
//  *			Dylan Frigerio,
//  *			Enzo Fernadez Rosas
//  *
//  *
//  * @copyright Copyright (c) 2023-2024
//  */

// #include "model.h"

// #include <stdint.h>

// #include <bitset>
// #include <iostream>

// #include "raylib.h"

// // Bitboard masks for board edges
// #define FILE_A 0x0101010101010101ULL  // Left column	(x=0)
// #define FILE_H 0x8080808080808080ULL  // Right column (x=7)
// #define RANK_1 0x00000000000000FFULL  // Top row		(y=0)
// #define RANK_8 0xFF00000000000000ULL  // Bottom row	(y=7)

// #define DIRECTION_COUNT 8

// namespace {

// const int8_t DIRECTIONS[8] = {
//     9,
//     8,
//     7,  // NW, N, NE
//     1,
//     /* O */ -1,  // W,    E
//     -7,
//     -8,
//     -9  // SW, S, SE
// };

// // Initial four pieces setup in the board center
// const int8_t initialPosition[2][2] = {{GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2 - 1, BOARD_SIZE / 2),
//                                        GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2, BOARD_SIZE / 2 - 1)},
//                                       {GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2 - 1, BOARD_SIZE / 2 - 1),
//                                        GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2, BOARD_SIZE / 2)}};

// /**
//  * @brief Returns the index (0�63) of the least significant set bit in a bitboard.
//  *
//  * This is used to scan for the next occupied or empty square efficiently.
//  */
// inline int8_t bitScanForward(uint64_t bb) {
// #if defined(_MSC_VER)  // MSVC (Windows)
//     unsigned long idx;
//     _BitScanForward64(&idx, bb);
//     return (int8_t)idx;
// #elif defined(__GNUC__) || defined(__clang__)  // GCC/Clang (Linux/macOS)
//     return __builtin_ctzll(bb);
// #else
//     // Fallback
//     int8_t idx = 0;
//     while (((bb >> idx) & 1ULL) == 0)
//         idx++;
//     return idx;
// #endif
// }

//     /**
//      * @brief Converts a bit index (0�63) to a 2D board coordinate (Square_t).
//      *
//      * @param n Bit index.
//      * @return Board coordinates (x, y).
//      */
//     inline Square_t transformBitToSquare(int8_t n) {
//         Square_t position;
//         position.x = n & 7;
//         position.y = n >> 3;
//         return position;
//     }

//     /**
//      * @brief Calculates the discs that would be flipped in one direction.
//      *
//      * Iterates along the given direction until a player disc is found,
//      * collecting opponent discs along the way. If the line is not closed,
//      * no discs are flipped.
//      *
//      * @param player Bitboard of current player.
//      * @param opponent Bitboard of opponent.
//      * @param startPos Starting empty square index.
//      * @param step Direction step offset.
//      * @param dir Direction index.
//      * @return Bitboard with flipped discs, or 0 if none.
//      */
//     uint64_t getFlipsInDirection(
//         uint64_t player, uint64_t opponent, int8_t startPos, int step, int dir) {
//         uint64_t flips = 0;
//         int curPos = startPos + step;

//         while (isSquareValid(curPos, dir)) {
//             uint64_t curBit = 1ULL << curPos;

//             if (opponent & curBit) {
//                 flips |= curBit;
//             } else if (player & curBit) {
//                 return flips;
//             } else {
//                 break;
//             }

//             curPos += step;
//         }

//         return 0ULL;
//     }

//     /**
//      * @brief Shift operations with edge masking
//      * These ensure pieces don't "wrap around" board edges
//      */
//     inline uint64_t shiftN(uint64_t bb) {
//         return bb >> 8;
//     }  // UP (y decreases)
//     inline uint64_t shiftS(uint64_t bb) {
//         return bb << 8;
//     }  // DOWN (y increases)
//     inline uint64_t shiftE(uint64_t bb) {
//         return (bb & ~FILE_H) << 1;
//     }  // RIGHT (mask H, prevent wrap to A)
//     inline uint64_t shiftW(uint64_t bb) {
//         return (bb & ~FILE_A) >> 1;
//     }  // LEFT (mask A, prevent wrap to H)
//     inline uint64_t shiftNE(uint64_t bb) {
//         return (bb & ~FILE_H) >> 7;
//     }  // UP-RIGHT
//     inline uint64_t shiftNW(uint64_t bb) {
//         return (bb & ~FILE_A) >> 9;
//     }  // UP-LEFT
//     inline uint64_t shiftSE(uint64_t bb) {
//         return (bb & ~FILE_H) << 9;
//     }  // DOWN-RIGHT
//     inline uint64_t shiftSW(uint64_t bb) {
//         return (bb & ~FILE_A) << 7;
//     }  // DOWN-LEFT

//     /**
//      * @brief Generates valid moves in one direction using kogge-stone propagation
//      *
//      * @param player Current player's pieces
//      * @param opponent Opponent's pieces
//      * @param shiftFunc Function pointer for directional shift
//      * @return Bitboard of valid moves in this direction
//      */
//     uint64_t generateMovesInDirection(uint64_t player,
//                                     uint64_t opponent,
//                                     uint64_t (*shiftFunc)(uint64_t)) {
//         uint64_t empty = ~(player | opponent);

//         // Start with opponent pieces adjacent to player pieces
//         uint64_t candidates = shiftFunc(player) & opponent;

//         // Propagate through opponent pieces (kogge-stone algorithm)
//         // Each iteration doubles the propagation distance
//         candidates |= shiftFunc(candidates) & opponent;
//         candidates |= shiftFunc(candidates) & opponent;
//         candidates |= shiftFunc(candidates) & opponent;
//         candidates |= shiftFunc(candidates) & opponent;
//         candidates |= shiftFunc(candidates) & opponent;
//         // 6 iterations = max propagation of 64 squares (2^6)

//         // One final shift into empty squares gives us valid moves
//         return shiftFunc(candidates) & empty;
//     }

//     /**
//      * @brief Fast parallel move generation using kogge-stone algorithm.
//      *
//      * This is 5-10x faster than checking each square individually.
//      * It uses bit-parallel operations to find all valid moves simultaneously.
//      *
//      * @param player Current player's bitboard
//      * @param opponent Opponent's bitboard
//      * @return Bitboard with all valid moves
//      */
//     uint64_t getValidMovesBitmap(uint64_t player, uint64_t opponent) {
//         uint64_t legal = 0ULL;

//         // Check all 8 directions
//         legal |= generateMovesInDirection(player, opponent, shiftN);
//         legal |= generateMovesInDirection(player, opponent, shiftS);
//         legal |= generateMovesInDirection(player, opponent, shiftE);
//         legal |= generateMovesInDirection(player, opponent, shiftW);
//         legal |= generateMovesInDirection(player, opponent, shiftNE);
//         legal |= generateMovesInDirection(player, opponent, shiftNW);
//         legal |= generateMovesInDirection(player, opponent, shiftSE);
//         legal |= generateMovesInDirection(player, opponent, shiftSW);

//         return legal;
//     }
// }  // namespace

// void initModel(GameModel& model) {
//     model.gameOver = true;

//     model.playerTime[0] = 0;
//     model.playerTime[1] = 0;

//     model.board.black = 0;
//     model.board.white = 0;
// }

// void startModel(GameModel& model) {
//     model.board.black = 0;
//     model.board.white = 0;

//     model.gameOver = false;

//     model.currentPlayer = PLAYER_BLACK;

//     model.playerTime[0] = 0;
//     model.playerTime[1] = 0;
//     model.turnTimer = GetTime();

//     SET_BIT(model.board.black, initialPosition[0][0]);
//     SET_BIT(model.board.black, initialPosition[0][1]);
//     SET_BIT(model.board.white, initialPosition[1][0]);
//     SET_BIT(model.board.white, initialPosition[1][1]);

//     /*std::cout << "Black: " << std::bitset<64>(model.board.black) << std::endl;
//     std::cout << "White: " << std::bitset<64>(model.board.white) << std::endl;*/
// }

// PlayerColor_t getCurrentPlayer(GameModel& model) {
//     return model.currentPlayer;
// }

// int getScore(GameModel& model, PlayerColor_t player) {
//     int score = 0;

//     uint64_t bitmap = (player == PLAYER_BLACK) ? model.board.black : model.board.white;

// #ifdef _MSC_VER
//     return __popcnt64(bitmap);
// #elif defined(__GNUC__) || defined(__clang__)
//     return __builtin_popcountll(bitmap);
// #else
//     // Fallback: Brian Kernighan
//     int count = 0;
//     while (bitmap) {
//         bitmap &= (bitmap - 1);
//         count++;
//     }
//     return count;
// #endif
// }

// double getTimer(GameModel& model, PlayerColor_t player) {
//     double turnTime = 0;

//     if (!model.gameOver && (player == model.currentPlayer))
//         turnTime = GetTime() - model.turnTimer;

//     return model.playerTime[player] + turnTime;
// }

// SquareState_t getBoardPiece(GameModel& model, int8_t n) {
//     //	if (!isSquareValid(square))
//     //		return PIECE_EMPTY;
//     return (GET_BIT(model.board.black, n)
//                 ? SQUARE_BLACK
//                 : (GET_BIT(model.board.white, n) ? SQUARE_WHITE : SQUARE_EMPTY));
// }

// void setBoardPiece(GameModel& model, int8_t n, SquareState_t piece) {
//     if (piece == SQUARE_BLACK) {
//         SET_BIT(model.board.black, n);
//         CLEAR_BIT(model.board.white, n);
//     } else if (piece == SQUARE_WHITE) {
//         SET_BIT(model.board.white, n);
//         CLEAR_BIT(model.board.black, n);
//     } else  // if(piece == SQUARE_EMPTY)
//     {
//         CLEAR_BIT(model.board.black, n);
//         CLEAR_BIT(model.board.white, n);
//     }
// }

// /**
//  * @brief Checks whether a square index is valid for movement in a given direction.
//  *
//  * Prevents moves that would wrap around the board edges.
//  *
//  * @param pos Square index (0�63).
//  * @param dir Direction index.
//  * @return True if the square is valid for stepping, false otherwise.
//  */
// bool isSquareValid(int8_t pos, int dir) {
//     if (pos < 0 || pos >= 64)
//         return false;

//     if (dir == NONE)
//         return true;
//     uint64_t bit = 1ULL << pos;

//     if ((dir == W || dir == NW || dir == SW) && (bit & FILE_A))
//         return false;
//     if ((dir == E || dir == NE || dir == SE) && (bit & FILE_H))
//         return false;
//     if ((dir == N || dir == NW || dir == NE) && (bit & RANK_1))
//         return false;
//     if ((dir == S || dir == SW || dir == SE) && (bit & RANK_8))
//         return false;

//     return true;
// }

// void getValidMoves(GameModel& model, Moves& validMoves) {
//     uint64_t player =
//         (getCurrentPlayer(model) == PLAYER_BLACK) ? model.board.black : model.board.white;
//     uint64_t opponent =
//         (getCurrentPlayer(model) == PLAYER_BLACK) ? model.board.white : model.board.black;
//     uint64_t validMovesBitmap = getValidMovesBitmap(player, opponent);  // Calculate once

//     while (validMovesBitmap) {
//         int8_t n = bitScanForward(validMovesBitmap);
//         validMoves.push_back(transformBitToSquare(n));
//         validMovesBitmap &= validMovesBitmap - 1;  // Clear LSB
//     }
// }

// bool playMove(GameModel& model, int8_t move) {
//     /// Get board state BEFORE placing piece
//     PlayerColor_t currentPlayer = getCurrentPlayer(model);
//     uint64_t player = (currentPlayer == PLAYER_BLACK) ? model.board.black : model.board.white;
//     uint64_t opponent = (currentPlayer == PLAYER_BLACK) ? model.board.white : model.board.black;

//     // Place the piece
//     SquareState_t piece = (currentPlayer == PLAYER_BLACK) ? SQUARE_BLACK : SQUARE_WHITE;
//     setBoardPiece(model, move, piece);

//     // Flip discs in all directions
//     for (int dir = 0; dir < DIRECTION_COUNT; dir++) {
//         uint64_t flips = getFlipsInDirection(player, opponent, move, DIRECTIONS[dir], dir);
//         if (flips) {
//             if (currentPlayer == PLAYER_BLACK) {
//                 model.board.black |= flips;
//                 model.board.white &= ~flips;
//             } else {
//                 model.board.white |= flips;
//                 model.board.black &= ~flips;
//             }
//         }
//     }

//     // Update timer
//     double currentTime = GetTime();
//     model.playerTime[model.currentPlayer] += currentTime - model.turnTimer;
//     model.turnTimer = currentTime;

//     // Swap player
//     model.currentPlayer = (model.currentPlayer == PLAYER_WHITE) ? PLAYER_BLACK : PLAYER_WHITE;

//     // Check if next player has valid moves
//     Moves validMoves;
//     getValidMoves(model, validMoves);

//     if (validMoves.size() == 0) {
//         // Swap player
//         model.currentPlayer = (model.currentPlayer == PLAYER_WHITE) ? PLAYER_BLACK : PLAYER_WHITE;

//         Moves validMoves;
//         getValidMoves(model, validMoves);

//         if (validMoves.size() == 0)
//             model.gameOver = true;
//     }

//     return true;
// }

/**
 * @brief Implements the Reversi game model
 * @author Marc S. Ressl
 * @modified:
 *			Agustin Valenzuela,
 *			Alex Petersen,
 *			Dylan Frigerio,
 *			Enzo Fernadez Rosas
 *
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "model.h"

#include <stdint.h>

#include <bitset>
#include <iostream>

#include "raylib.h"

// Bitboard masks for board edges
#define FILE_A 0x0101010101010101ULL  // Left column	(x=0)
#define FILE_H 0x8080808080808080ULL  // Right column (x=7)
#define RANK_1 0x00000000000000FFULL  // Top row		(y=0)
#define RANK_8 0xFF00000000000000ULL  // Bottom row	(y=7)

#define DIRECTION_COUNT 8

namespace {

const int8_t DIRECTIONS[8] = {
    9,
    8,
    7,  // NW, N, NE
    1,
    /* O */ -1,  // W,    E
    -7,
    -8,
    -9  // SW, S, SE
};

// Initial four pieces setup in the board center
const int8_t initialPosition[2][2] = {{GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2 - 1, BOARD_SIZE / 2),
                                       GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2, BOARD_SIZE / 2 - 1)},
                                      {GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2 - 1, BOARD_SIZE / 2 - 1),
                                       GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2, BOARD_SIZE / 2)}};

/**
 * @brief Calculates the discs that would be flipped in one direction.
 *
 * Iterates along the given direction until a player disc is found,
 * collecting opponent discs along the way. If the line is not closed,
 * no discs are flipped.
 *
 * @param player Bitboard of current player.
 * @param opponent Bitboard of opponent.
 * @param startPos Starting empty square index.
 * @param step Direction step offset.
 * @param dir Direction index.
 * @return Bitboard with flipped discs, or 0 if none.
 */
uint64_t getFlipsInDirection(
    uint64_t player, uint64_t opponent, int8_t startPos, int step, int dir) {
    uint64_t flips = 0;
    int curPos = startPos + step;

    while (isSquareValid(curPos, dir)) {
        uint64_t curBit = 1ULL << curPos;

        if (opponent & curBit) {
            flips |= curBit;
        } else if (player & curBit) {
            return flips;
        } else {
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
}  // UP (y decreases)
inline uint64_t shiftS(uint64_t bb) {
    return bb << 8;
}  // DOWN (y increases)
inline uint64_t shiftE(uint64_t bb) {
    return (bb & ~FILE_H) << 1;
}  // RIGHT (mask H, prevent wrap to A)
inline uint64_t shiftW(uint64_t bb) {
    return (bb & ~FILE_A) >> 1;
}  // LEFT (mask A, prevent wrap to H)
inline uint64_t shiftNE(uint64_t bb) {
    return (bb & ~FILE_H) >> 7;
}  // UP-RIGHT
inline uint64_t shiftNW(uint64_t bb) {
    return (bb & ~FILE_A) >> 9;
}  // UP-LEFT
inline uint64_t shiftSE(uint64_t bb) {
    return (bb & ~FILE_H) << 9;
}  // DOWN-RIGHT
inline uint64_t shiftSW(uint64_t bb) {
    return (bb & ~FILE_A) << 7;
}  // DOWN-LEFT

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
                                  uint64_t (*shiftFunc)(uint64_t)) {
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

}  // namespace

// ============================================================================
// NEW: Core bitboard operations (now public)
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

uint64_t calculateFlips(uint64_t player, uint64_t opponent, int8_t move) {
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

int8_t bitScanForward(uint64_t bb) {
#if defined(_MSC_VER)  // MSVC (Windows)
    unsigned long idx;
    _BitScanForward64(&idx, bb);
    return (int8_t)idx;
#elif defined(__GNUC__) || defined(__clang__)  // GCC/Clang (Linux/macOS)
    return __builtin_ctzll(bb);
#else
    // Fallback
    int8_t idx = 0;
    while (((bb >> idx) & 1ULL) == 0)
        idx++;
    return idx;
#endif
}

// ============================================================================
// NEW: AI helper functions
// ============================================================================

void getValidMovesAI(const Board_t& board, PlayerColor_t player, std::vector<Move_t>& moves) {
    moves.clear();

    uint64_t playerBB = (player == PLAYER_BLACK) ? board.black : board.white;
    uint64_t opponentBB = (player == PLAYER_BLACK) ? board.white : board.black;
    uint64_t validMovesBitmap = getValidMovesBitmap(playerBB, opponentBB);

    while (validMovesBitmap) {
        int8_t n = bitScanForward(validMovesBitmap);
        moves.push_back(n);
        validMovesBitmap &= validMovesBitmap - 1;  // Clear LSB
    }
}

bool hasValidMoves(const Board_t& board, PlayerColor_t player) {
    uint64_t playerBB = (player == PLAYER_BLACK) ? board.black : board.white;
    uint64_t opponentBB = (player == PLAYER_BLACK) ? board.white : board.black;
    return getValidMovesBitmap(playerBB, opponentBB) != 0ULL;
}

bool isTerminal(const Board_t& board, PlayerColor_t player) {
    // Terminal if current player can't move AND opponent can't move
    if (hasValidMoves(board, player))
        return false;

    PlayerColor_t opponent = (player == PLAYER_BLACK) ? PLAYER_WHITE : PLAYER_BLACK;
    return !hasValidMoves(board, opponent);
}

int getScoreDiff(const Board_t& board, PlayerColor_t player) {
    int playerScore = countBits((player == PLAYER_BLACK) ? board.black : board.white);
    int opponentScore = countBits((player == PLAYER_BLACK) ? board.white : board.black);
    return playerScore - opponentScore;
}

BoardState_t makeMove(Board_t& board, PlayerColor_t& currentPlayer, Move_t move) {
    // Save current state
    BoardState_t state;
    state.black = board.black;
    state.white = board.white;
    state.player = currentPlayer;

    // Calculate flips
    uint64_t player = (currentPlayer == PLAYER_BLACK) ? board.black : board.white;
    uint64_t opponent = (currentPlayer == PLAYER_BLACK) ? board.white : board.black;
    uint64_t flips = calculateFlips(player, opponent, move);

    // Apply move and flips
    uint64_t moveBit = 1ULL << move;
    if (currentPlayer == PLAYER_BLACK) {
        board.black |= moveBit | flips;
        board.white &= ~flips;
    } else {
        board.white |= moveBit | flips;
        board.black &= ~flips;
    }

    // Swap player
    currentPlayer = (currentPlayer == PLAYER_BLACK) ? PLAYER_WHITE : PLAYER_BLACK;

    return state;
}

void unmakeMove(Board_t& board, PlayerColor_t& currentPlayer, const BoardState_t& state) {
    board.black = state.black;
    board.white = state.white;
    currentPlayer = state.player;
}

// ============================================================================
// Existing game functions (some refactored to use new helpers)
// ============================================================================

void initModel(GameModel& model) {
    model.gameOver = true;

    model.playerTime[0] = 0;
    model.playerTime[1] = 0;

    model.board.black = 0;
    model.board.white = 0;
}

void startModel(GameModel& model) {
    model.board.black = 0;
    model.board.white = 0;

    model.gameOver = false;

    model.currentPlayer = PLAYER_BLACK;

    model.playerTime[0] = 0;
    model.playerTime[1] = 0;
    model.turnTimer = GetTime();

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
    return countBits(bitmap);  // REFACTORED: Now uses countBits()
}

double getTimer(GameModel& model, PlayerColor_t player) {
    double turnTime = 0;

    if (!model.gameOver && (player == model.currentPlayer))
        turnTime = GetTime() - model.turnTimer;

    return model.playerTime[player] + turnTime;
}

SquareState_t getBoardPiece(GameModel& model, int8_t n) {
    return (GET_BIT(model.board.black, n)
                ? SQUARE_BLACK
                : (GET_BIT(model.board.white, n) ? SQUARE_WHITE : SQUARE_EMPTY));
}

void setBoardPiece(GameModel& model, int8_t n, SquareState_t piece) {
    if (piece == SQUARE_BLACK) {
        SET_BIT(model.board.black, n);
        CLEAR_BIT(model.board.white, n);
    } else if (piece == SQUARE_WHITE) {
        SET_BIT(model.board.white, n);
        CLEAR_BIT(model.board.black, n);
    } else  // if(piece == SQUARE_EMPTY)
    {
        CLEAR_BIT(model.board.black, n);
        CLEAR_BIT(model.board.white, n);
    }
}

bool isSquareValid(int8_t pos, int dir) {
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

void getValidMoves(GameModel& model, Moves& validMoves) {
    validMoves.clear();

    uint64_t player =
        (getCurrentPlayer(model) == PLAYER_BLACK) ? model.board.black : model.board.white;
    uint64_t opponent =
        (getCurrentPlayer(model) == PLAYER_BLACK) ? model.board.white : model.board.black;
    uint64_t validMovesBitmap = getValidMovesBitmap(player, opponent);

    while (validMovesBitmap) {
        int8_t n = bitScanForward(validMovesBitmap);

        // Convert bit index to Square_t
        Square_t square;
        square.x = n & 7;
        square.y = n >> 3;

        validMoves.push_back(square);
        validMovesBitmap &= validMovesBitmap - 1;  // Clear LSB
    }
}

bool playMove(GameModel& model, int8_t move) {
    // Get board state BEFORE placing piece
    PlayerColor_t currentPlayer = getCurrentPlayer(model);
    uint64_t player = (currentPlayer == PLAYER_BLACK) ? model.board.black : model.board.white;
    uint64_t opponent = (currentPlayer == PLAYER_BLACK) ? model.board.white : model.board.black;

    // REFACTORED: Use calculateFlips()
    uint64_t flips = calculateFlips(player, opponent, move);

    // Apply move and flips
    uint64_t moveBit = 1ULL << move;
    if (currentPlayer == PLAYER_BLACK) {
        model.board.black |= moveBit | flips;
        model.board.white &= ~flips;
    } else {
        model.board.white |= moveBit | flips;
        model.board.black &= ~flips;
    }

    // Update timer
    double currentTime = GetTime();
    model.playerTime[model.currentPlayer] += currentTime - model.turnTimer;
    model.turnTimer = currentTime;

    // Swap player
    model.currentPlayer = (model.currentPlayer == PLAYER_WHITE) ? PLAYER_BLACK : PLAYER_WHITE;

    // Check if next player has valid moves
    Moves validMoves;
    getValidMoves(model, validMoves);

    if (validMoves.size() == 0) {
        // Swap player back
        model.currentPlayer = (model.currentPlayer == PLAYER_WHITE) ? PLAYER_BLACK : PLAYER_WHITE;

        Moves validMoves2;
        getValidMoves(model, validMoves2);

        if (validMoves2.size() == 0)
            model.gameOver = true;
    }

    return true;
}
