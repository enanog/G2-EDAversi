/**
 * @brief Implements the Reversi game controller with AI threading
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include <algorithm>
#include <thread>
#include <atomic>
#include <mutex>
#include <iostream>
#include <memory>

#include "raylib.h"
#include "ai/ai_factory.h"
#include "view.h"
#include "controller.h"

 // Threading synchronization
static std::thread aiThread;
static std::atomic<bool> aiThreadRunning(false);
static std::mutex aiMutex;

// Current AI instance
static std::unique_ptr<AIInterface> currentAI;

// Menu state
static bool menuActive = true;

/**
 * @brief AI worker function running in separate thread
 * @param modelPtr Pointer to game model for analysis
 */
void aiWorkerFunction(GameModel* modelPtr) {
    std::cout << "[AI Thread] Started thinking..." << std::endl;

    // Create thread-safe copy of model
    GameModel localModel;
    {
        std::lock_guard<std::mutex> lock(aiMutex);
        localModel = *modelPtr;
    }

    // Verify AI instance exists
    if (!currentAI) {
        std::cerr << "[AI Thread] ERROR: No AI instance!" << std::endl;
        std::lock_guard<std::mutex> lock(aiMutex);
        modelPtr->aiThinking = false;
        aiThreadRunning = false;
        return;
    }

    std::cout << "[AI Thread] Using: " << currentAI->getName() << std::endl;

    // Polymorphic AI call
    Move_t bestMove = currentAI->getBestMove(localModel);

    std::cout << "[AI Thread] Found move: " << (int)bestMove << std::endl;

    // Store result with thread safety
    {
        std::lock_guard<std::mutex> lock(aiMutex);
        modelPtr->aiMove = bestMove;
        modelPtr->aiThinking = false;
    }

    std::cout << "[AI Thread] Finished!" << std::endl;
    aiThreadRunning = false;
}

/**
 * @brief Starts AI analysis in background thread
 */
void startAIThinking(GameModel& model) {
    std::cout << "[Main] Starting AI thinking..." << std::endl;

    if (!currentAI) {
        std::cerr << "[Main] ERROR: No AI initialized! Call initializeAI() first." << std::endl;
        return;
    }

    if (aiThread.joinable()) {
        aiThread.join();
    }

    model.aiThinking = true;
    model.aiMove = MOVE_NONE;
    aiThreadRunning = true;

    aiThread = std::thread(aiWorkerFunction, &model);

    std::cout << "[Main] AI thread launched!" << std::endl;
}

/**
 * @brief Checks for and applies completed AI move
 * @return true if AI move was applied
 */
bool checkAndApplyAIMove(GameModel& model) {
    bool isThinking;
    Move_t move;

    {
        std::lock_guard<std::mutex> lock(aiMutex);
        isThinking = model.aiThinking;
        move = model.aiMove;
    }

    if (!isThinking && move != MOVE_NONE) {
        std::cout << "[Main] AI finished! Applying move: " << (int)move << std::endl;

        playMove(model, move);
        model.aiMove = MOVE_NONE;

        if (aiThread.joinable()) {
            aiThread.join();
        }

        std::cout << "[Main] Move applied!" << std::endl;
        return true;
    }

    return false;
}

// AI management implementation
void initializeAI(AIDifficulty difficulty) {
    std::cout << "[Controller] Initializing AI: "
        << AIFactory::getDifficultyName(difficulty) << std::endl;

    currentAI = AIFactory::createAI(difficulty);

    if (currentAI) {
        std::cout << "[Controller] AI ready: " << currentAI->getName() << std::endl;
    }
    else {
        std::cerr << "[Controller] ERROR: Failed to create AI!" << std::endl;
    }
}

void changeAIDifficulty(AIDifficulty difficulty) {
    if (aiThreadRunning) {
        std::cerr << "[Controller] Cannot change AI while thinking!" << std::endl;
        return;
    }

    if (aiThread.joinable()) {
        aiThread.join();
    }

    initializeAI(difficulty);
}

const char* getCurrentAIName() {
    if (currentAI) {
        return currentAI->getName();
    }
    return "No AI";
}

/**
 * @brief Main controller loop handling input and game flow
 */
bool updateView(GameModel& model) {
    if (WindowShouldClose()) {
        if (aiThread.joinable()) {
            aiThread.join();
        }
        return false;
    }

    // Handle main menu if active
    if (menuActive) {
        drawMainMenu();

        // Process difficulty selection clicks
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (isMousePointerOverAIEasyButton()) {
                std::cout << "[Menu] Easy AI selected\n";
                initializeAI(AIDifficulty::AI_EASY);
                menuActive = false;
            }
            else if (isMousePointerOverAINormalButton()) {
                std::cout << "[Menu] Normal AI selected\n";
                initializeAI(AIDifficulty::AI_NORMAL);
                menuActive = false;
            }
            else if (isMousePointerOverAIHardButton()) {
                std::cout << "[Menu] Hard AI selected\n";
                initializeAI(AIDifficulty::AI_HARD);
                menuActive = false;
            }
            else if (isMousePointerOverAIExtremeButton()) {
                std::cout << "[Menu] Extreme AI selected\n";
                initializeAI(AIDifficulty::AI_EXTREME);
                menuActive = false;
            }
        }

        return true;
    }

    // Normal gameplay flow
    if (model.gameOver) {
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
        // Human player turn
        if (IsMouseButtonPressed(0)) {
            Move_t move = getMoveOnMousePointer();

            if (move != MOVE_NONE) {
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
        // AI player turn
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