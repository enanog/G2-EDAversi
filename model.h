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

// ============================================================================
// Bitboard manipulation macros
// ============================================================================

#define SET_BIT(bitmap, n) ((bitmap) |= (1ULL << (n)))
#define GET_BIT(bitmap, n) (((bitmap) >> (n)) & 1ULL)
#define CLEAR_BIT(bitmap, n) ((bitmap) &= ~(1ULL << (n)))

// ============================================================================
// Direction constants
// ============================================================================

enum { NW, N, NE, W, E, SW, S, SE, NONE };

// ============================================================================
// Core types
// ============================================================================

/**
 * @brief Move representation (bit index 0-63)
 */
typedef int8_t Move_t;
#define MOVE_NONE ((Move_t)-1)
#define MOVE_PASS ((Move_t)-2)

/**
 * @brief Vector of moves
 */
typedef std::vector<Move_t> MoveList;

/**
 * @brief Piece/Square state
 */
typedef enum {
    STATE_BLACK = 0,
    STATE_WHITE = 1,
    STATE_EMPTY = 2
} PieceState_t;

/**
 * @brief Player color (same values as piece state)
 */
typedef PieceState_t PlayerColor_t;
#define PLAYER_BLACK STATE_BLACK
#define PLAYER_WHITE STATE_WHITE

/**
 * @brief Board state (two bitboards)
 */
typedef struct {
    uint64_t black;
    uint64_t white;
} Board_t;

/**
 * @brief Board snapshot for make/unmake operations
 */
typedef struct {
    uint64_t black;
    uint64_t white;
    PlayerColor_t player;
} BoardState_t;

/**
 * @brief Main game model
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

    // Turn pass notification
    bool showPassMessage;
    PlayerColor_t passedPlayer;
    double passMessageStartTime;
};

// ============================================================================
// Coordinate conversion utilities (inline for zero overhead)
// ============================================================================

/**
 * @brief Converts (x,y) to Move_t
 */
inline Move_t coordsToMove(int8_t x, int8_t y) {
    return x + (y << 3);
}

/**
 * @brief Gets X coordinate from Move_t (0-7)
 */
inline int8_t getMoveX(Move_t move) {
    return move & 7;
}

/**
 * @brief Gets Y coordinate from Move_t (0-7)
 */
inline int8_t getMoveY(Move_t move) {
    return move >> 3;
}

/**
 * @brief Checks if move is valid position
 */
inline bool isMoveInBounds(Move_t move) {
    return move >= 0 && move < 64;
}

// ============================================================================
// Game model operations
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
 * @brief Gets current elapsed time for a player
 * This is thread-safe and updates continuously
 * @param model The game model
 * @param player The player
 * @return Total elapsed time in seconds
 */
double getTimer(GameModel& model, PlayerColor_t player);

/**
 * @brief Return a model's piece.
 *
 * @param model  The game model.
 * @param square The index of the square.
 * @return The piece at the square.
 */
PieceState_t getBoardPiece(GameModel& model, Move_t square);

/**
 * @brief Sets a model's piece.
 *
 * @param model The game model.
 * @param move The index of the square.
 * @param piece The piece to be set
 */
void setBoardPiece(GameModel& model, Move_t move, PieceState_t piece);

/**
 * @brief Checks whether a square is within the board.
 * @param move The square as Move_t (0-63)
 * @param dir Direction to check (NW,N,NE,W,E,SW,S,SE)
 * @return True or false
 */
bool isSquareValid(Move_t move, int dir);

/**
 * @brief Gets valid moves for current player
 * @param model Game model
 * @param validMoves Output vector of moves (Move_t) 
 */
void getValidMoves(GameModel& model, MoveList& validMoves);

/**
 * @brief Plays a move
 * @param model Game model
 * @param move Move to play (0-63)
 * @return True if move accepted
 */
bool playMove(GameModel& model, Move_t move);

// ============================================================================
// Core bitboard operations
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
Move_t bitScanForward(uint64_t bb);

// ============================================================================
// AI/Search functions
// ============================================================================

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

/**
 * @brief Gets move count without allocating vector (for mobility evaluation).
 *
 * @param board The board state
 * @param player The player
 * @return Number of valid moves
 */
int getMoveCount(const Board_t& board, PlayerColor_t player);

/**
 * @brief Checks if a move is valid (without generating full move list).
 *
 * @param board The board state
 * @param player The player
 * @param move The move to check
 * @return True if move is legal
 */
bool isMoveValid(const Board_t& board, PlayerColor_t player, Move_t move);

// ============================================================================
// Inline helper functions (zero overhead)
// ============================================================================

/**
 * @brief Gets the opponent of a player.
 *
 * @param player Current player
 * @return Opponent player
 */
inline PlayerColor_t getOpponent(PlayerColor_t player) {
    return (PlayerColor_t)(player ^ 1);
}

/**
 * @brief Gets player's bitboard from board.
 *
 * @param board The board state
 * @param player The player
 * @return Player's bitboard
 */
inline uint64_t getPlayerBitboard(const Board_t& board, PlayerColor_t player) {
    return (player == PLAYER_BLACK) ? board.black : board.white;
}

/**
 * @brief Gets opponent's bitboard from board.
 *
 * @param board The board state
 * @param player Current player
 * @return Opponent's bitboard
 */
inline uint64_t getOpponentBitboard(const Board_t& board, PlayerColor_t player) {
    return (player == PLAYER_BLACK) ? board.white : board.black;
}

/**
 * @brief Gets empty squares bitboard.
 *
 * @param board The board state
 * @return Empty squares bitboard
 */
inline uint64_t getEmptyBitboard(const Board_t& board) {
    return ~(board.black | board.white);
}

/**
 * @brief Counts total discs on board.
 *
 * @param board The board state
 * @return Number of discs (0-64)
 */
inline int getDiscCount(const Board_t& board) {
    return countBits(board.black | board.white);
}

/**
 * @brief Counts empty squares on board.
 *
 * @param board The board state
 * @return Number of empty squares (0-64)
 */
inline int getEmptyCount(const Board_t& board) {
    return 64 - getDiscCount(board);
}

/**
 * @brief Checks if a square is empty.
/**
 * @brief Checks if a square is empty.
 *
 * @param board The board state
 * @param move Square position
 * @return True if square is empty
 */
inline bool isEmpty(const Board_t& board, Move_t move) {
    uint64_t occupied = board.black | board.white;
    return (occupied & (1ULL << move)) == 0;
}

/**
 * @brief Checks if a square is occupied by player.
 *
 * @param board The board state
 * @param player The player
 * @param move Square position
 * @return True if player occupies the square
 */
inline bool hasDisc(const Board_t& board, PlayerColor_t player, Move_t move) {
    uint64_t playerBB = getPlayerBitboard(board, player);
    return (playerBB & (1ULL << move)) != 0;
}

/**
 * @brief Copies board state.
 *
 * @param dest Destination board
 * @param src Source board
 */
inline void copyBoard(Board_t& dest, const Board_t& src) {
    dest.black = src.black;
    dest.white = src.white;
}

/**
 * @brief Compares two boards for equality.
 *
 * @param a First board
 * @param b Second board
 * @return True if boards are identical
 */
inline bool boardsEqual(const Board_t& a, const Board_t& b) {
    return (a.black == b.black) && (a.white == b.white);
}

// ============================================================================
// Bitboard constants
// ============================================================================

#define CORNERS    0x8100000000000081ULL  // a1,h1,a8,h8
#define X_SQUARES  0x4200000000004200ULL  // b2,g2,b7,g7
#define C_SQUARES  0x0042000000004200ULL  // Adjacent to corners
#define EDGES      0x7E8181818181817EULL  // Edges (no corners)
#define INNER      0x007E7E7E7E7E7E00ULL  // Center 6x6
#define CENTER_4   0x0000001818000000ULL  // d4,e4,d5,e5

/**
 * @brief Counts discs in a specific region (using mask).
 *
 * @param board The board state
 * @param player The player
 * @param mask Region mask (e.g., CORNERS, EDGES)
 * @return Number of player's discs in masked region
 */
inline int countRegion(const Board_t& board, PlayerColor_t player, uint64_t mask) {
    return countBits(getPlayerBitboard(board, player) & mask);
}

/**
 * @brief Gets all corner discs for a player.
 *
 * @param board The board state
 * @param player The player
 * @return Number of corners held by player
 */
inline int getCornerCount(const Board_t& board, PlayerColor_t player) {
    return countRegion(board, player, CORNERS);
}

#endif