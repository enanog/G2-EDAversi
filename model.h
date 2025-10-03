/**
 * @brief Implements the Reversi game model
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>
#include <cstdint>
#include <vector>

#define BOARD_SIZE 8

// (0, 0) -> Top-Left
// (7, 7) -> Bottom-Right

#define GET_SQUARE_BIT_INDEX(x, y) ( (x) + ((y) << 3) )
#define SET_BIT(bitmap, n) ( (bitmap) |= (1ULL << (n)) )
#define GET_BIT(bitmap, n) ( ((bitmap) >> (n)) & 1ULL )
#define SET_BIT(bitmap, n) ( (bitmap) |= (1ULL << (n)) )
#define CLEAR_BIT(bitmap, n) ( (bitmap) &= ~(1ULL << (n)) )
#define TOGGLE_BIT(bitmap, n) ( (bitmap) ^= (1ULL << (n)) )
#define TOGGLE_MASK(bitmap, mask) ( (bitmap) ^= mask )
#define TOGGLE_PIECE_COLOR(board, n) TOGGLE_BIT((board).black, (n)) ; TOGGLE_BIT((board).white, (n))

typedef struct
{
	int8_t x;
	int8_t y;
} Square_t;

#define GAME_INVALID_SQUARE {(int8_t)-1, (int8_t)-1}

typedef enum
{
	SQUARE_BLACK,
	SQUARE_WHITE,
	SQUARE_EMPTY
} SquareState_t;

typedef enum
{
	PLAYER_BLACK = SQUARE_BLACK,
	PLAYER_WHITE = SQUARE_WHITE
} PlayerColor_t;

typedef struct
{
	uint64_t black;
	uint64_t white;
} Board_t;

struct GameModel
{
	bool gameOver;

	double playerTime[2];
	double turnTimer;

	Board_t board;
	PlayerColor_t currentPlayer;
	PlayerColor_t humanPlayer;
};

typedef std::vector<Square_t> Moves;

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
bool isSquareValid(int8_t pos);

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

#endif
