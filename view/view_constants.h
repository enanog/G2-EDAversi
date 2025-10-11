/**
 * @brief Constants and configuration for view rendering
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef VIEW_CONSTANTS_H
#define VIEW_CONSTANTS_H

#include "../model.h"

#define GAME_NAME "EDAversi"

// Window configuration
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

// Board rendering constants
#define SQUARE_SIZE 80
#define SQUARE_PADDING 1.5F
#define SQUARE_CONTENT_OFFSET (SQUARE_PADDING)
#define SQUARE_CONTENT_SIZE (SQUARE_SIZE - 2 * SQUARE_PADDING)

#define PIECE_CENTER (SQUARE_SIZE / 2)
#define PIECE_RADIUS (SQUARE_SIZE * 80 / 100 / 2)

#define BOARD_X 40
#define BOARD_Y 40
#define BOARD_CONTENT_SIZE (BOARD_SIZE * SQUARE_SIZE)

#define OUTERBORDER_X (BOARD_X - OUTERBORDER_PADDING)
#define OUTERBORDER_Y (BOARD_Y - OUTERBORDER_PADDING)
#define OUTERBORDER_PADDING 40
#define OUTERBORDER_WIDTH 10
#define OUTERBORDER_SIZE (BOARD_CONTENT_SIZE + 2 * OUTERBORDER_PADDING)

// Text rendering constants
#define TITLE_FONT_SIZE 72
#define SUBTITLE_FONT_SIZE 36

// Info panel positioning
#define INFO_CENTERED_X (OUTERBORDER_SIZE + (WINDOW_WIDTH - OUTERBORDER_SIZE) / 2)

#define INFO_TITLE_Y (WINDOW_HEIGHT / 2)

#define INFO_WHITE_SCORE_Y (WINDOW_HEIGHT * 1 / 4 - SUBTITLE_FONT_SIZE / 2)
#define INFO_WHITE_TIME_Y (WINDOW_HEIGHT * 1 / 4 + SUBTITLE_FONT_SIZE / 2)

#define INFO_BLACK_SCORE_Y (WINDOW_HEIGHT * 3 / 4 - SUBTITLE_FONT_SIZE / 2)
#define INFO_BLACK_TIME_Y (WINDOW_HEIGHT * 3 / 4 + SUBTITLE_FONT_SIZE / 2)

// Button configuration
#define INFO_BUTTON_WIDTH 280
#define INFO_BUTTON_HEIGHT 64

#define INFO_PLAYBLACK_BUTTON_X INFO_CENTERED_X
#define INFO_PLAYBLACK_BUTTON_Y (WINDOW_HEIGHT * 1 / 8)

#define INFO_PLAYWHITE_BUTTON_X INFO_CENTERED_X
#define INFO_PLAYWHITE_BUTTON_Y (WINDOW_HEIGHT * 7 / 8)

// AI difficulty selection button positions
#define AI_BUTTON_X (WINDOW_WIDTH * 1 / 2)
#define AI_EASY_BUTTON_Y (WINDOW_HEIGHT * 3 / 8)
#define AI_NORMAL_BUTTON_Y (WINDOW_HEIGHT * 4 / 8)
#define AI_HARD_BUTTON_Y (WINDOW_HEIGHT * 5 / 8)
#define AI_EXTREME_BUTTON_Y (WINDOW_HEIGHT * 6 / 8)

#endif // VIEW_CONSTANTS_H