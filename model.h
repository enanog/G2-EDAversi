/**
 * @brief Implements the Reversi game model
 * @author Marc S. Ressl
 * @modified:
 *			Agustin Valenzuela,
 *			Alex Petersen,
 *			Dylan Frigerio,
 *			Enzo Fernadez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef MODEL_H
#define MODEL_H

#include <cstdint>
#include <vector>

#define BOARD_SIZE 8

/**
 * BITMAP REPRESENTATION:
 * The board uses 64-bit integers where each bit is a square (index 0-63)
 *
 *      A  B  C  D  E  F  G  H
 * 1 |  0  1  2  3  4  5  6  7 |
 * 2 |  8  9 10 11 12 13 14 15 |
 * 3 | 16 17 18 19 20 21 22 23 |
 * 4 | 24 25 26 27 28 29 30 31 |
 * 5 | 32 33 34 35 36 37 38 39 |
 * 6 | 40 41 42 43 44 45 46 47 |
 * 7 | 48 49 50 51 52 53 54 55 |
 * 8 | 56 57 58 59 60 61 62 63 |
 *
 * Initial position: Black={27,36}, White={28,35}
 */

// ---------------------------------------------------------------------------
// Basic types and macros
// ---------------------------------------------------------------------------

/** Move representation (board square index 0..63) */
typedef int8_t Move_t;
#define MOVE_NONE ((Move_t)-1)
#define MOVE_PASS ((Move_t)-2)

/** Vector type for moves */
typedef std::vector<Move_t> MoveList;

/** Piece / square state */
typedef enum {
    STATE_BLACK = 0,
    STATE_WHITE = 1,
    STATE_EMPTY = 2
} PieceState_t;

/** Player color type (same values as piece state) */
typedef PieceState_t PlayerColor_t;
#define PLAYER_BLACK STATE_BLACK
#define PLAYER_WHITE STATE_WHITE

/** Two-bitboard representation (black + white) */
typedef struct {
    uint64_t black;
    uint64_t white;
} Board_t;

/** Snapshot of board for make/unmake operations */
typedef struct {
    uint64_t black;
    uint64_t white;
    PlayerColor_t player;
} BoardState_t;

/**
 * @brief Main game model structure
 *
 * This struct is POD-like and stores the full state of a game. We keep
 * members public for simple access; functions operate on this struct.
 */
struct GameModel {
    Board_t board;
    PlayerColor_t currentPlayer;
    PlayerColor_t humanPlayer;

    bool gameOver;
    double playerTime[2];
    double turnStartTime;

    // AI threading state
    bool aiThinking;
    Move_t aiMove;

	bool playedPass;
    bool pauseTimers;
};

// ---------------------------------------------------------------------------
// Bitboard helper macros (low-level helpers used by implementation)
// ---------------------------------------------------------------------------

#define SET_BIT(bitmap, n) ((bitmap) |= (1ULL << (n)))
#define GET_BIT(bitmap, n) (((bitmap) >> (n)) & 1ULL)
#define CLEAR_BIT(bitmap, n) ((bitmap) &= ~(1ULL << (n)))

// Direction indices
enum { NW, N, NE, W, E, SW, S, SE, NONE };

// ---------------------------------------------------------------------------
// Coordinate conversion utilities (inline, zero-overhead)
// ---------------------------------------------------------------------------

/** Convert x,y coordinates to Move_t index (0..63) */
inline Move_t coordsToMove(int8_t x, int8_t y) {
    return static_cast<Move_t>(x + (y << 3));
}

/** Get X coordinate (0..7) from Move_t */
inline int8_t getMoveX(Move_t move) {
    return static_cast<int8_t>(move & 7);
}

/** Get Y coordinate (0..7) from Move_t */
inline int8_t getMoveY(Move_t move) {
    return static_cast<int8_t>(move >> 3);
}

/** Check if move is in valid bounds (0..63) */
inline bool isMoveInBounds(Move_t move) {
    return move >= 0 && move < 64;
}

// ---------------------------------------------------------------------------
// Game model operations
// ---------------------------------------------------------------------------

/**
 * @brief Initialize model to a safe default (no active game).
 *
 * Sets sensible defaults and clears the board. This does not start a game.
 */
void initModel(GameModel& model);

/**
 * @brief Start a new game and set initial position and timers.
 *
 * Resets board, current player and timing state.
 */
void startModel(GameModel& model);

/**
 * @brief Return the model's current player.
 *
 * Read-only accessor; accepts const reference.
 */
PlayerColor_t getCurrentPlayer(const GameModel& model);

/**
 * @brief Return the player's score (number of discs).
 *
 * Read-only accessor; accepts const reference.
 */
int getScore(const GameModel& model, PlayerColor_t player);

/**
 * @brief Return elapsed time for a player in seconds.
 *
 * This function is intended to be a read-only accessor. If the timer
 * logic updates shared state, it must still preserve observable behavior.
 */
double getTimer(const GameModel& model, PlayerColor_t player);

/**
 * @brief Return the piece at a given square.
 *
 * Read-only accessor.
 */
PieceState_t getBoardPiece(const GameModel& model, Move_t square);

/**
 * @brief Set a piece at a given square.
 *
 * Mutates the model's board bitboards.
 */
void setBoardPiece(GameModel& model, Move_t move, PieceState_t piece);

/**
 * @brief Check whether a square in a given direction is valid (edge tests).
 *
 * Utility used by directional checks.
 */
bool isSquareValid(Move_t move, int dir);

/**
 * @brief Populate validMoves with all legal moves for the current player.
 *
 * This function does not mutate the model (it only reads state).
 */
void getValidMoves(const GameModel& model, MoveList& validMoves);

/**
 * @brief Play a move on the model (applies flips, updates timers and current player).
 *
 * Returns true when the move was accepted and applied.
 */
bool playMove(GameModel& model, Move_t move);

// ---------------------------------------------------------------------------
// Core bitboard operations (pure functions)
// ---------------------------------------------------------------------------

uint64_t getValidMovesBitmap(uint64_t player, uint64_t opponent);
uint64_t calculateFlips(uint64_t player, uint64_t opponent, int8_t move);

int countBits(uint64_t bitmap);
Move_t bitScanForward(uint64_t bb);

// ---------------------------------------------------------------------------
// AI / search helpers (pure / read-only where possible)
// ---------------------------------------------------------------------------

void getValidMovesAI(const Board_t& board, PlayerColor_t player, std::vector<Move_t>& moves);
bool hasValidMoves(const Board_t& board, PlayerColor_t player);
bool isTerminal(const Board_t& board, PlayerColor_t player);
int getScoreDiff(const Board_t& board, PlayerColor_t player);

/**
 * @brief Make/unmake lightweight board moves for search.
 *
 * These are low-level utilities used by search algorithms. They modify
 * the given Board_t and PlayerColor_t references.
 */
BoardState_t makeMove(Board_t& board, PlayerColor_t& currentPlayer, Move_t move);
void unmakeMove(Board_t& board, PlayerColor_t& currentPlayer, const BoardState_t& state);

int getMoveCount(const Board_t& board, PlayerColor_t player);
bool isMoveValid(const Board_t& board, PlayerColor_t player, Move_t move);

// ---------------------------------------------------------------------------
// Inline helpers (implemented in header for speed)
// ---------------------------------------------------------------------------

inline PlayerColor_t getOpponent(PlayerColor_t player) {
    return static_cast<PlayerColor_t>(player ^ 1);
}

inline uint64_t getPlayerBitboard(const Board_t& board, PlayerColor_t player) {
    return (player == PLAYER_BLACK) ? board.black : board.white;
}

inline uint64_t getOpponentBitboard(const Board_t& board, PlayerColor_t player) {
    return (player == PLAYER_BLACK) ? board.white : board.black;
}

inline uint64_t getEmptyBitboard(const Board_t& board) {
    return ~(board.black | board.white);
}

inline int getDiscCount(const Board_t& board) {
    return countBits(board.black | board.white);
}

inline int getEmptyCount(const Board_t& board) {
    return 64 - getDiscCount(board);
}

inline bool isEmpty(const Board_t& board, Move_t move) {
    uint64_t occupied = board.black | board.white;
    return (occupied & (1ULL << move)) == 0;
}

inline bool hasDisc(const Board_t& board, PlayerColor_t player, Move_t move) {
    uint64_t playerBB = getPlayerBitboard(board, player);
    return (playerBB & (1ULL << move)) != 0;
}

inline void copyBoard(Board_t& dest, const Board_t& src) {
    dest.black = src.black;
    dest.white = src.white;
}

inline bool boardsEqual(const Board_t& a, const Board_t& b) {
    return (a.black == b.black) && (a.white == b.white);
}

// ---------------------------------------------------------------------------
// Bitboard masks (constants)
// ---------------------------------------------------------------------------

#define CORNERS    0x8100000000000081ULL
#define X_SQUARES  0x4200000000004200ULL
#define C_SQUARES  0x0042000000004200ULL
#define EDGES      0x7E8181818181817EULL
#define INNER      0x007E7E7E7E7E7E00ULL
#define CENTER_4   0x0000001818000000ULL

inline int countRegion(const Board_t& board, PlayerColor_t player, uint64_t mask) {
    return countBits(getPlayerBitboard(board, player) & mask);
}

inline int getCornerCount(const Board_t& board, PlayerColor_t player) {
    return countRegion(board, player, CORNERS);
}

#endif // MODEL_H