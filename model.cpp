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

const int8_t initialPosition[2][2] = 
{
	{
		GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2 - 1, BOARD_SIZE / 2),
		GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2, BOARD_SIZE / 2 - 1)
	},
	{
		GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2 - 1, BOARD_SIZE / 2 - 1),
		GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2 , BOARD_SIZE / 2 ) 
	}
};

namespace
{
	inline Square_t transformBitToSquare(int8_t n) 
	{
		Square_t position;
		position.x = n & 7;
		position.y = n >> 3;
		return position;
	}
}

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

	std::cout << "Black: " << std::bitset<64>(model.board.black) << std::endl;
	std::cout << "White: " << std::bitset<64>(model.board.white) << std::endl;
	printf("%d %d %d %d", GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2 - 1, BOARD_SIZE / 2),
		GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2, BOARD_SIZE / 2 - 1), 

		GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2 - 1, BOARD_SIZE / 2 - 1),
		GET_SQUARE_BIT_INDEX(BOARD_SIZE / 2, BOARD_SIZE / 2));
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

bool isSquareValid(Square_t square)
{
	return (square.x >= 0) &&
		   (square.x < BOARD_SIZE) &&
		   (square.y >= 0) &&
		   (square.y < BOARD_SIZE);
}

void getValidMoves(GameModel &model, Moves &validMoves)
{
	// To-do: your code goes here...

	for (int n = 0; n < (sizeof(model.board.black) * 8); n++)
	{
		// +++ TEST
		// Lists all empty squares...
		if (getBoardPiece(model, n) == SQUARE_EMPTY)
			validMoves.push_back(transformBitToSquare(n));
		// --- TEST
	}
}

bool playMove(GameModel &model, int8_t move)
{
	// Set game piece
	SquareState_t piece = (getCurrentPlayer(model) == PLAYER_BLACK)? SQUARE_BLACK : SQUARE_WHITE;

	setBoardPiece(model, move, piece);

	// To-do: your code goes here...

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

		//Moves validMoves;
		//getValidMoves(model, validMoves);

		if (validMoves.size() == 0)
			model.gameOver = true;
	}

	return true;
}