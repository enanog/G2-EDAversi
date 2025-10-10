/**
 * @brief Implements the Reversi game controller with AI threading
 * @author Marc S. Ressl
 * @modified:
 *			Agustin Valenzuela,
 *			Alex Petersen,
 *			Dylan Frigerio,
 *			Enzo Fernadez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include <algorithm>
#include <thread>
#include <atomic>
#include <mutex>
#include <iostream>

#include "raylib.h"

#include "ai.h"
#include "view.h"
#include "controller.h"

 // Threading globals
static std::thread aiThread;
static std::atomic<bool> aiThreadRunning(false);
static std::mutex aiMutex;

/**
 * @brief AI worker function - runs in separate thread
 */
void aiWorkerFunction(GameModel* modelPtr) {
    std::cout << "[AI Thread] Started thinking..." << std::endl;

    // Create a deep copy of the model for thread safety
    GameModel localModel;
    {
        std::lock_guard<std::mutex> lock(aiMutex);
        localModel = *modelPtr;
    }

    std::cout << "[AI Thread] Calculating best move..." << std::endl;

    // Calculate best move (this takes time)
    Move_t bestMove = getBestMove(localModel);

    std::cout << "[AI Thread] Found move: " << (int)bestMove << std::endl;

    // Lock and write result
    {
        std::lock_guard<std::mutex> lock(aiMutex);
        modelPtr->aiMove = bestMove;
        modelPtr->aiThinking = false;
    }

    std::cout << "[AI Thread] Finished!" << std::endl;
    aiThreadRunning = false;
}

/**
 * @brief Starts AI thinking in background thread
 */
void startAIThinking(GameModel& model) {
    std::cout << "[Main] Starting AI thinking..." << std::endl;

    // If there's an old thread, make sure it's joined
    if (aiThread.joinable()) {
        std::cout << "[Main] Joining old thread..." << std::endl;
        aiThread.join();
    }

    model.aiThinking = true;
    model.aiMove = MOVE_NONE;
    aiThreadRunning = true;

    // Launch new AI thread
    aiThread = std::thread(aiWorkerFunction, &model);

    std::cout << "[Main] AI thread launched!" << std::endl;
}

/**
 * @brief Checks if AI has finished and applies move if ready
 */
bool checkAndApplyAIMove(GameModel& model) {
    // Check if AI has finished (thread-safe read)
    bool isThinking;
    Move_t move;

    {
        std::lock_guard<std::mutex> lock(aiMutex);
        isThinking = model.aiThinking;
        move = model.aiMove;
    }

    // If AI finished thinking and has a valid move
    if (!isThinking && move != MOVE_NONE) {
        std::cout << "[Main] AI finished! Applying move: " << (int)move << std::endl;

        // Apply the move
        playMove(model, move);
        model.aiMove = MOVE_NONE;

        // Join the thread
        if (aiThread.joinable()) {
            aiThread.join();
        }

        std::cout << "[Main] Move applied!" << std::endl;
        return true;
    }

    return false;
}

bool updateView(GameModel& model) {
    if (WindowShouldClose()) {
        // Clean up AI thread before exiting
        if (aiThread.joinable()) {
            aiThread.join();
        }
        return false;
    }

    if (model.gameOver) {
        // Game over screen
        if (IsMouseButtonPressed(0)) {
            if (isMousePointerOverPlayBlackButton()) {
                std::cout << "[Main] Starting new game - Human plays BLACK" << std::endl;
                model.humanPlayer = PLAYER_BLACK;
                startModel(model);
            }
            else if (isMousePointerOverPlayWhiteButton()) {
                std::cout << "[Main] Starting new game - Human plays WHITE" << std::endl;
                model.humanPlayer = PLAYER_WHITE;
                startModel(model);
            }
        }
    }
    else if (model.currentPlayer == model.humanPlayer) {
        // Human turn
        if (IsMouseButtonPressed(0)) {
            Move_t move = getMoveOnMousePointer();

            if (move != MOVE_NONE) {
                // Check if move is valid
                MoveList validMoves;
                getValidMoves(model, validMoves);

                auto it = std::find(validMoves.begin(), validMoves.end(), move);
                if (it != validMoves.end()) {
                    std::cout << "[Main] Human plays move: " << (int)move << std::endl;
                    playMove(model, move);
                }
            }
        }
    }
    else {
        // AI turn
        if (checkAndApplyAIMove(model)) {
            std::cout << "[Main] AI move processed, next turn." << std::endl;
        }
        else if (!model.aiThinking) {
            std::cout << "[Main] AI turn detected, starting AI..." << std::endl;
            startAIThinking(model);
        }
    }

    // Toggle fullscreen with Alt+Enter
    if ((IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) &&
        IsKeyPressed(KEY_ENTER)) {
        ToggleFullscreen();
    }

    drawView(model);

    return true;
}