/**
 * @brief Implements the Reversi game model
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "raylib.h"

#include <iostream>
#include <bitset>
#include "model.h"
#include <stdint.h>

#define FILE_A 0x0101010101010101ULL // Left column	(x=0)
#define FILE_H 0x8080808080808080ULL // Right column (x=7)
#define RANK_1 0x00000000000000FFULL // Top row		(y=0)
#define RANK_8 0xFF00000000000000ULL // Bottom row	(y=7)

#define DIRECTION_COUNT 8

namespace
{
	enum { NW, N, NE, W, E, SW, S, SE };

	const int8_t DIRECTIONS[8] =
	{
		9,    8,    7,
		1, /* O */ -1,
	   -7,   -8,   -9
	};

	const int8_t initialPosition[2][2] =
	{
		{
			GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2 - 1, BOARD_SIZE / 2),
			GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2, BOARD_SIZE / 2 - 1)
		},
		{
			GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2 - 1, BOARD_SIZE / 2 - 1),
			GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2 , BOARD_SIZE / 2)
		}
	};


	inline int8_t bitScanForward(uint64_t bb) {
#if defined(_MSC_VER)  // MSVC (Windows)
		unsigned long idx;
		_BitScanForward64(&idx, bb);
		return (int8_t)idx;
#elif defined(__GNUC__) || defined(__clang__)  // GCC/Clang (Linux/macOS)
		return __builtin_ctzll(bb);
#else
		// Fallback
		int8_t idx = 0;
		while (((bb >> idx) & 1ULL) == 0) idx++;
		return idx;
#endif
	}

	inline Square_t transformBitToSquare(int8_t n)
	{
		Square_t position;
		position.x = n & 7;
		position.y = n >> 3;
		return position;
	}

	bool isSquareValid(int8_t pos, int dir)
	{
		if (pos < 0 || pos >= 64) return false;
		uint64_t bit = 1ULL << pos;

		if ((dir == W || dir == NW || dir == SW) && (bit & FILE_A)) return false;
		if ((dir == E || dir == NE || dir == SE) && (bit & FILE_H)) return false;
		if ((dir == N || dir == NW || dir == NE) && (bit & RANK_1)) return false;
		if ((dir == S || dir == SW || dir == SE) && (bit & RANK_8)) return false;

		return true;
	}

	uint64_t getFlipsInDirection(uint64_t player, uint64_t opponent, int8_t startPos, int step, int dir)
	{
		uint64_t flips = 0;
		int curPos = startPos + step;

		while (isSquareValid(curPos, dir))
		{
			uint64_t curBit = 1ULL << curPos;

			if (opponent & curBit)
			{
				flips |= curBit; 
			}
			else if (player & curBit)
			{
				return flips; 
			}
			else
			{
				break; 
			}

			curPos += step;
		}

		return 0ULL; 
	}

	uint64_t getValidMovesBitmap(GameModel& model)
	{
		PlayerColor_t currentPlayer = getCurrentPlayer(model);
		uint64_t player = (currentPlayer == PLAYER_BLACK) ? model.board.black : model.board.white;
		uint64_t opponent = (currentPlayer == PLAYER_BLACK) ? model.board.white : model.board.black;
		uint64_t emptySquares = ~(player | opponent);
		uint64_t validMovesBitmap = 0ULL;

		while (emptySquares)
		{
			int8_t pos = bitScanForward(emptySquares);
			uint64_t bit = 1ULL << pos;

			for (int dir = 0; dir < DIRECTION_COUNT; dir++)
			{
				if (uint64_t flips = getFlipsInDirection(player, opponent, pos, DIRECTIONS[dir], dir))
				{
					validMovesBitmap |= bit;
					break; 
				}
			}

			emptySquares &= emptySquares - 1;
		}

		return validMovesBitmap;
	}
};


void initModel(GameModel &model)
{
	model.gameOver = true;

	model.playerTime[0] = 0;
	model.playerTime[1] = 0;

	model.board.black = 0;
	model.board.white = 0;
}

void startModel(GameModel &model)
{
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

	/*std::cout << "Black: " << std::bitset<64>(model.board.black) << std::endl;
	std::cout << "White: " << std::bitset<64>(model.board.white) << std::endl;*/
}

PlayerColor_t getCurrentPlayer(GameModel &model)
{
	return model.currentPlayer;
}

int getScore(GameModel &model, PlayerColor_t player)
{
	int score = 0;

	uint64_t bitmap = (player == PLAYER_BLACK) ? model.board.black : model.board.white;

#ifdef _MSC_VER
	return __popcnt64(bitmap);
#elif defined(__GNUC__) || defined(__clang__)
	return __builtin_popcountll(bitmap);
#else
	// Fallback: Brian Kernighan
	int count = 0;
	while (bitmap)
	{
		bitmap &= (bitmap - 1);
		count++;
	}
	return count;
#endif
}

double getTimer(GameModel &model, PlayerColor_t player)
{
	double turnTime = 0;

	if (!model.gameOver && (player == model.currentPlayer))
		turnTime = GetTime() - model.turnTimer;

	return model.playerTime[player] + turnTime;
}

SquareState_t getBoardPiece(GameModel &model, int8_t n)
{
//	if (!isSquareValid(square))
//		return PIECE_EMPTY;
	return (GET_BIT(model.board.black, n) ? SQUARE_BLACK :
		   (GET_BIT(model.board.white, n) ? SQUARE_WHITE : SQUARE_EMPTY));
}

void setBoardPiece(GameModel &model, int8_t n, SquareState_t piece)
{
	if(piece == SQUARE_BLACK)
	{
		SET_BIT(model.board.black, n);
		CLEAR_BIT(model.board.white, n);
	}
	else if(piece == SQUARE_WHITE)
	{
		SET_BIT(model.board.white, n);
		CLEAR_BIT(model.board.black, n);
	}
	else // if(piece == SQUARE_EMPTY)
	{
		CLEAR_BIT(model.board.black, n);
		CLEAR_BIT(model.board.white, n);
	}
}

bool isSquareValid(int8_t pos)
{
	if (pos < 0 || pos >= 64) return false;
	return true;
}

void getValidMoves(GameModel &model, Moves& validMoves)
{
	for (int n = 0; n < (sizeof(model.board.black) * 8); n++)
	{
		// +++ TEST
		// Lists all empty squares...
		uint64_t validMovesBitmap = getValidMovesBitmap(model);
		if (GET_BIT(validMovesBitmap, n))
			validMoves.push_back(transformBitToSquare(n));
		// --- TEST
	}
}

bool playMove(GameModel &model, int8_t move)
{
	// Set game piece
	SquareState_t piece = (getCurrentPlayer(model) == PLAYER_BLACK) ? SQUARE_BLACK : SQUARE_WHITE;
	uint64_t player = (getCurrentPlayer(model) == PLAYER_BLACK) ? model.board.black : model.board.white;
	uint64_t opponent = (getCurrentPlayer(model) == PLAYER_BLACK) ? model.board.white : model.board.black;

	setBoardPiece(model, move, piece);

	for (int dir = 0; dir < DIRECTION_COUNT; dir++)
	{
		uint64_t flips = getFlipsInDirection(player, opponent, move, DIRECTIONS[dir], dir);
		if (flips)
		{
			if (getCurrentPlayer(model) == PLAYER_BLACK)
			{
				model.board.black |= flips;
				model.board.white &= ~flips;
			}
			else
			{
				model.board.white |= flips;
				model.board.black &= ~flips;
			}
		}
	}

	// Update timer
	double currentTime = GetTime();
	model.playerTime[model.currentPlayer] += currentTime - model.turnTimer;
	model.turnTimer = currentTime;

	// Swap player
	model.currentPlayer =
		(model.currentPlayer == PLAYER_WHITE)
		? PLAYER_BLACK
		: PLAYER_WHITE;

	// Game over?
	Moves validMoves;
	getValidMoves(model, validMoves);

	if (validMoves.size() == 0)
	{
		// Swap player
		model.currentPlayer =
			(model.currentPlayer == PLAYER_WHITE)
			? PLAYER_BLACK
			: PLAYER_WHITE;

		Moves validMoves;
		getValidMoves(model, validMoves);

		if (validMoves.size() == 0)
			model.gameOver = true;
	}

	return true;
}