// /**
//  * @brief Implements the Reversi game model
//  * @author Marc S. Ressl
//  * @modified:
//  *			Agustin Valenzuela,
//  *			Alex Petersen,
//  *			Dylan Frigerio,
//  *			Enzo Fernadez Rosas
//  *
//  * @copyright Copyright (c) 2023-2024
//  */

// #ifndef MODEL_H
// #define MODEL_H

// #include <stdint.h>

// #include <cstdint>
// #include <vector>

// #define BOARD_SIZE 8

// // Board coordinate mapping:
// // (0, 0) -> Top-Left
// // (7, 7) -> Bottom-Right

// /**
//  * @brief Converts (x,y) coordinates into a bit index (0�63).
//  */
// #define GET_SQUARE_BIT_INDEX(x, y) ((x) + ((y) << 3))

// /**
//  * @brief Sets a bit in a bitboard.
//  */
// #define SET_BIT(bitmap, n) ((bitmap) |= (1ULL << (n)))

// /**
//  * @brief Retrieves a bit from a bitboard.
//  */
// #define GET_BIT(bitmap, n) (((bitmap) >> (n)) & 1ULL)

// /**
//  * @brief Clears a bit in a bitboard.
//  */
// #define CLEAR_BIT(bitmap, n) ((bitmap) &= ~(1ULL << (n)))

// enum { NW, N, NE, W, E, SW, S, SE, NONE };

// /**
//  * @brief Represents a square position in 2D coordinates.
//  */
// typedef struct {
//     int8_t x;
//     int8_t y;
// } Square_t;

// #define GAME_INVALID_SQUARE {(int8_t)-1, (int8_t)-1}

// /**
//  * @brief State of a square on the board.
//  */
// typedef enum { SQUARE_BLACK, SQUARE_WHITE, SQUARE_EMPTY } SquareState_t;

// /**
//  * @brief Identifies player color (mapped to square states).
//  */
// typedef enum { PLAYER_BLACK = SQUARE_BLACK, PLAYER_WHITE = SQUARE_WHITE } PlayerColor_t;

// /**
//  * @brief Holds two bitboards (black and white pieces).
//  */
// typedef struct {
//     uint64_t black;
//     uint64_t white;
// } Board_t;

// /**
//  * @brief Main game model storing board, players, and timers.
//  */
// struct GameModel {
//     bool gameOver;

//     double playerTime[2];
//     double turnTimer;

//     Board_t board;
//     PlayerColor_t currentPlayer;
//     PlayerColor_t humanPlayer;
// };

// typedef std::vector<Square_t> Moves;

// /**
//  * @brief Initializes a game model.
//  *
//  * @param model The game model.
//  */
// void initModel(GameModel& model);

// /**
//  * @brief Starts a game.
//  *
//  * @param model The game model.
//  */
// void startModel(GameModel& model);

// /**
//  * @brief Returns the model's current player.
//  *
//  * @param model The game model.
//  * @return PLAYER_WHITE or PLAYER_BLACK.
//  */
// PlayerColor_t getCurrentPlayer(GameModel& model);

// /**
//  * @brief Returns the model's current score.
//  *
//  * @param model The game model.
//  * @param player The player (PLAYER_WHITE or PLAYER_BLACK).
//  * @return The score.
//  */
// int getScore(GameModel& model, PlayerColor_t player);

// /**
//  * @brief Returns the game timer for a player.
//  *
//  * @param model The game model.
//  * @param player The player (PLAYER_WHITE or PLAYER_BLACK).
//  * @return The time in seconds.
//  */
// double getTimer(GameModel& model, PlayerColor_t player);

// /**
//  * @brief Return a model's piece.
//  *
//  * @param model The game model.
//  * @param square The square.
//  * @return The piece at the square.
//  */
// SquareState_t getBoardPiece(GameModel& model, int8_t n);

// /**
//  * @brief Sets a model's piece.
//  *
//  * @param model The game model.
//  * @param square The square.
//  * @param piece The piece to be set
//  */
// void setBoardPiece(GameModel& model, int8_t n, SquareState_t piece);

// /**
//  * @brief Checks whether a square is within the board.
//  *
//  * @param square The square.
//  * @return True or false.
//  */
// bool isSquareValid(int8_t pos, int dir);

// /**
//  * @brief Returns a list of valid moves for the current player.
//  *
//  * @param model The game model.
//  * @param validMoves A list that receives the valid moves.
//  */
// void getValidMoves(GameModel& model, Moves& validMoves);

// /**
//  * @brief Plays a move.
//  *
//  * @param model The game model.
//  * @param square The move.
//  * @return Move accepted.
//  */
// bool playMove(GameModel& model, int8_t move);

// #endif

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

#include <stdint.h>

#include <cstdint>
#include <vector>

#define BOARD_SIZE 8

// Board coordinate mapping:
// (0, 0) -> Top-Left
// (7, 7) -> Bottom-Right

/**
 * @brief Converts (x,y) coordinates into a bit index (0–63).
 */
#define GET_SQUARE_BIT_INDEX(x, y) ((x) + ((y) << 3))

/**
 * @brief Sets a bit in a bitboard.
 */
#define SET_BIT(bitmap, n) ((bitmap) |= (1ULL << (n)))

/**
 * @brief Retrieves a bit from a bitboard.
 */
#define GET_BIT(bitmap, n) (((bitmap) >> (n)) & 1ULL)

/**
 * @brief Clears a bit in a bitboard.
 */
#define CLEAR_BIT(bitmap, n) ((bitmap) &= ~(1ULL << (n)))

enum { NW, N, NE, W, E, SW, S, SE, NONE };

/**
 * @brief Represents a square position in 2D coordinates.
 */
typedef struct {
    int8_t x;
    int8_t y;
} Square_t;

#define GAME_INVALID_SQUARE {(int8_t)-1, (int8_t)-1}

/**
 * @brief State of a square on the board.
 */
typedef enum { SQUARE_BLACK, SQUARE_WHITE, SQUARE_EMPTY } SquareState_t;

/**
 * @brief Identifies player color (mapped to square states).
 */
typedef enum { PLAYER_BLACK = SQUARE_BLACK, PLAYER_WHITE = SQUARE_WHITE } PlayerColor_t;

/**
 * @brief Holds two bitboards (black and white pieces).
 */
typedef struct {
    uint64_t black;
    uint64_t white;
} Board_t;

/**
 * @brief Main game model storing board, players, and timers.
 */
struct GameModel {
    bool gameOver;

    double playerTime[2];
    double turnTimer;

    Board_t board;
    PlayerColor_t currentPlayer;
    PlayerColor_t humanPlayer;
};

typedef std::vector<Square_t> Moves;

// ============================================================================
// NEW: AI-specific types and constants
// ============================================================================

/**
 * @brief Lightweight move representation (bit index 0-63)
 */
typedef int8_t Move_t;
#define MOVE_NONE ((Move_t) - 1)
#define MOVE_PASS ((Move_t) - 2)

/**
 * @brief Board state snapshot for make/unmake operations
 */
typedef struct {
    uint64_t black;
    uint64_t white;
    PlayerColor_t player;
} BoardState_t;

// ============================================================================
// Existing game functions (unchanged interface)
// ============================================================================

/**
 * @brief Initializes a game model.
 *
 * @param model The game model.
 */
void initModel(GameModel& model);

/**
 * @brief Starts a game.
 *
 * @param model The game model.
 */
void startModel(GameModel& model);

/**
 * @brief Returns the model's current player.
 *
 * @param model The game model.
 * @return PLAYER_WHITE or PLAYER_BLACK.
 */
PlayerColor_t getCurrentPlayer(GameModel& model);

/**
 * @brief Returns the model's current score.
 *
 * @param model The game model.
 * @param player The player (PLAYER_WHITE or PLAYER_BLACK).
 * @return The score.
 */
int getScore(GameModel& model, PlayerColor_t player);

/**
 * @brief Returns the game timer for a player.
 *
 * @param model The game model.
 * @param player The player (PLAYER_WHITE or PLAYER_BLACK).
 * @return The time in seconds.
 */
double getTimer(GameModel& model, PlayerColor_t player);

/**
 * @brief Return a model's piece.
 *
 * @param model The game model.
 * @param square The square.
 * @return The piece at the square.
 */
SquareState_t getBoardPiece(GameModel& model, int8_t n);

/**
 * @brief Sets a model's piece.
 *
 * @param model The game model.
 * @param square The square.
 * @param piece The piece to be set
 */
void setBoardPiece(GameModel& model, int8_t n, SquareState_t piece);

/**
 * @brief Checks whether a square is within the board.
 *
 * @param square The square.
 * @return True or false.
 */
bool isSquareValid(int8_t pos, int dir);

/**
 * @brief Returns a list of valid moves for the current player.
 *
 * @param model The game model.
 * @param validMoves A list that receives the valid moves.
 */
void getValidMoves(GameModel& model, Moves& validMoves);

/**
 * @brief Plays a move.
 *
 * @param model The game model.
 * @param square The move.
 * @return Move accepted.
 */
bool playMove(GameModel& model, int8_t move);

// ============================================================================
// NEW: Core bitboard operations (extracted from internals)
// ============================================================================

/**
 * @brief Generates valid moves bitmap using Kogge-Stone algorithm.
 *
 * @param player Current player's bitboard
 * @param opponent Opponent's bitboard
 * @return Bitboard with all valid moves
 */
uint64_t getValidMovesBitmap(uint64_t player, uint64_t opponent);

/**
 * @brief Calculates all discs that would be flipped by a move.
 *
 * @param player Current player's bitboard
 * @param opponent Opponent's bitboard
 * @param move Move position (bit index 0-63)
 * @return Bitboard of all flipped discs
 */
uint64_t calculateFlips(uint64_t player, uint64_t opponent, int8_t move);

/**
 * @brief Counts set bits in a bitboard (population count).
 *
 * @param bitmap The bitboard
 * @return Number of set bits
 */
int countBits(uint64_t bitmap);

/**
 * @brief Returns the index of the least significant set bit.
 *
 * @param bb Bitboard (must be non-zero)
 * @return Bit index (0-63)
 */
int8_t bitScanForward(uint64_t bb);

// ============================================================================
// NEW: AI helper functions (lightweight operations)
// ============================================================================

/**
 * @brief Converts Square_t to Move_t (bit index).
 */
inline Move_t squareToMove(Square_t square) {
    if (square.x < 0 || square.y < 0)
        return MOVE_NONE;
    return GET_SQUARE_BIT_INDEX(square.x, square.y);
}

/**
 * @brief Converts Move_t to Square_t.
 */
inline Square_t moveToSquare(Move_t move) {
    if (move < 0)
        return GAME_INVALID_SQUARE;
    Square_t sq;
    sq.x = move & 7;
    sq.y = move >> 3;
    return sq;
}

/**
 * @brief Gets valid moves as Move_t array (for AI).
 *
 * @param board The board state
 * @param player Current player
 * @param moves Output vector of moves
 */
void getValidMovesAI(const Board_t& board, PlayerColor_t player, std::vector<Move_t>& moves);

/**
 * @brief Checks if a player has any valid moves.
 *
 * @param board The board state
 * @param player The player to check
 * @return True if player has at least one valid move
 */
bool hasValidMoves(const Board_t& board, PlayerColor_t player);

/**
 * @brief Checks if the position is terminal (game over).
 *
 * @param board The board state
 * @param player Current player
 * @return True if neither player can move
 */
bool isTerminal(const Board_t& board, PlayerColor_t player);

/**
 * @brief Returns score differential (current player - opponent).
 *
 * @param board The board state
 * @param player Current player
 * @return Score difference (positive favors current player)
 */
int getScoreDiff(const Board_t& board, PlayerColor_t player);

/**
 * @brief Makes a move on the board (lightweight, no game logic).
 *
 * @param board The board state (modified)
 * @param currentPlayer Current player (will be swapped)
 * @param move The move to make
 * @return Previous board state for unmake
 */
BoardState_t makeMove(Board_t& board, PlayerColor_t& currentPlayer, Move_t move);

/**
 * @brief Unmakes a move (restores previous state).
 *
 * @param board The board state (modified)
 * @param currentPlayer Current player (modified)
 * @param state The saved state to restore
 */
void unmakeMove(Board_t& board, PlayerColor_t& currentPlayer, const BoardState_t& state);

#endif
