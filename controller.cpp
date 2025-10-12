/**
 * @brief Implements the Reversi game controller with AI threading
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernadez Rosas
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
#include "view/view.h"
#include "view/view_constants.h"
#include "view/settings_overlay.h"
#include "view/ui_components.h"
#include "controller.h"

static std::thread aiThread;
static std::atomic<bool> aiThreadRunning(false);
static std::mutex aiMutex;

// Current AI instance
static std::unique_ptr<AIInterface> currentAI;

// Menu / app state
enum GameState {
    STATE_MAIN_MENU,
    STATE_AI_SETTINGS_MENU,
    STATE_PLAYING
};

static GameState currentState = STATE_MAIN_MENU;
static bool showSettingsOverlay = false;

// Game configuration
static AIDifficulty currentDifficulty = AIDifficulty::AI_NORMAL;
static int currentNodeLimit = 1000;
static bool aiEnabled = false;  // false => 1v1 mode

// UI temporary selection
// - settingsPendingSelection: visual selection while overlay open (-1 = none -> use currentDifficulty)
// - scheduledDifficulty: difficulty scheduled to apply once AI finishes thinking (-1 = none)
static int settingsPendingSelection = -1;
static int scheduledDifficulty = -1;

// ---------------------------------------------------------------------------
// AI thread worker
// ---------------------------------------------------------------------------

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

    // Polymorphic AI call (may be expensive / blocking)
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

    // Ensure previously launched thread has finished
    if (aiThread.joinable()) {
        aiThread.join();
    }

    model.aiThinking = true;
    model.aiMove = MOVE_NONE;
    aiThreadRunning = true;

    // Launch worker
    aiThread = std::thread(aiWorkerFunction, &model);

    std::cout << "[Main] AI thread launched!" << std::endl;
}

static void cancelAIIfRunning(GameModel& model) {
    if (aiThreadRunning) {
        std::cout << "[Controller] Cancelling AI move..." << std::endl;
        {
            std::lock_guard<std::mutex> lock(aiMutex);
            model.aiThinking = false;
            model.aiMove = MOVE_NONE;
        }
        aiThreadRunning = false;
        if (aiThread.joinable()) {
            aiThread.join();
        }
    }
}

// ---------------------------------------------------------------------------
// Apply scheduled difficulty (if any) when safe
// ---------------------------------------------------------------------------

static void applyScheduledDifficultyIfAny() {
    // Called from main thread when it's safe to change AI (aiThreadStopped).
    if (scheduledDifficulty != -1) {
        AIDifficulty pd = static_cast<AIDifficulty>(scheduledDifficulty);
        std::cout << "[Controller] Applying scheduled difficulty: "
            << AIFactory::getDifficultyName(pd) << std::endl;
        scheduledDifficulty = -1;
        changeAIDifficulty(pd);
    }
}

// ---------------------------------------------------------------------------
// Check & apply completed AI move
// ---------------------------------------------------------------------------

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

    // If AI finished and produced a move (not MOVE_NONE), apply it
    if (!isThinking && move != MOVE_NONE) {
        std::cout << "[Main] AI finished! Applying move: " << (int)move << std::endl;

        bool moveApplied = playMove(model, move);

        // Clear stored move
        {
            std::lock_guard<std::mutex> lock(aiMutex);
            model.aiMove = MOVE_NONE;
        }

        // Ensure thread joined
        if (aiThread.joinable()) {
            aiThread.join();
        }

        // After thread has stopped, apply any scheduled difficulty change
        // (safe because aiThread has been joined and aiThreadRunning should be false)
        applyScheduledDifficultyIfAny();

        if (moveApplied) {
            std::cout << "[Main] Move applied successfully!" << std::endl;
            std::cout << "[Main] GameOver: " << model.gameOver
                << ", Current player: " << (model.currentPlayer == PLAYER_BLACK ? "BLACK" : "WHITE")
                << ", ShowPass: " << model.playedPass << std::endl;
        }
        else {
            std::cout << "[Main] WARNING: Move was rejected by playMove!" << std::endl;
        }

        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// AI management helpers
// ---------------------------------------------------------------------------

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
    // If the AI is currently thinking, schedule the change instead of forcing it.
    if (aiThreadRunning) {
        std::cerr << "[Controller] AI is thinking, scheduling difficulty change after finish.\n";
        scheduledDifficulty = static_cast<int>(difficulty);
        return;
    }

    // Ensure thread (if any) is joined before replacing AI instance
    if (aiThread.joinable()) {
        aiThread.join();
    }

    currentDifficulty = difficulty;
    initializeAI(difficulty);
}

const char* getCurrentAIName() {
    if (currentAI) {
        return currentAI->getName();
    }
    return "No AI";
}

std::string getDifficultyString(AIDifficulty difficulty) {
    switch (difficulty) {
    case AIDifficulty::AI_EASY:    return "Easy";
    case AIDifficulty::AI_NORMAL:  return "Normal";
    case AIDifficulty::AI_HARD:    return "Hard";
    case AIDifficulty::AI_EXTREME: return "Extreme";
    default: return "Unknown";
    }
}

// ---------------------------------------------------------------------------
// Menu / Input handlers
// ---------------------------------------------------------------------------

/**
 * @brief Handles main menu interactions
 */
void handleMainMenu(GameModel& model) {
    drawMainMenu();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (isMousePointerOverMenu1v1Button()) {
            std::cout << "[Menu] 1v1 mode selected (no AI)\n";
            aiEnabled = false;
            model.humanPlayer = PLAYER_BLACK; // default to black unless player chooses differently
            startModel(model);
            currentState = STATE_PLAYING;
        }
        else if (isMousePointerOverMenu1vAIButton()) {
            std::cout << "[Menu] 1 vs AI mode selected\n";
            aiEnabled = true;
            if (!currentAI) {
                initializeAI(currentDifficulty);
            }
            currentState = STATE_PLAYING;
        }
        else if (isMousePointerOverMenuSettingsButton()) {
            std::cout << "[Menu] Opening AI settings\n";
            currentState = STATE_AI_SETTINGS_MENU;
        }
    }
}

/**
 * @brief Handles AI settings menu interactions
 */
void handleAISettingsMenu() {
    drawAIDifficultyMenu();
    static int8_t selectedOption = -1;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (isMousePointerOverAIEasyButton()) {
            std::cout << "[Settings] Easy AI selected\n";
            selectedOption = 0;
        }
        else if (isMousePointerOverAINormalButton()) {
            std::cout << "[Settings] Normal AI selected\n";
            selectedOption = 1;
        }
        else if (isMousePointerOverAIHardButton()) {
            std::cout << "[Settings] Hard AI selected\n";
            selectedOption = 2;
        }
        else if (isMousePointerOverAIExtremeButton()) {
            std::cout << "[Settings] Extreme AI selected\n";
            selectedOption = 3;
        }
        else if (isMousePointerOverBackToMenuButton()) {
            std::cout << "[Settings] Back to main menu (settings cancelled)\n";
            currentState = STATE_MAIN_MENU;
            selectedOption = -1;
        }
        else if (isMousePointerOverContinueToMenuButton()) {
            std::cout << "[Settings] Settings confirmed, returning to main menu\n";
            if (selectedOption != -1) {
                changeAIDifficulty(static_cast<AIDifficulty>(selectedOption));
            }
            currentState = STATE_MAIN_MENU;
            selectedOption = -1;
        }
    }
}

/**
 * @brief Handles in-game settings overlay (visual selection + scheduling)
 *
 * Note: Uses file-scoped static 'settingsPendingSelection' and 'scheduledDifficulty'
 * to avoid adding fields to GameModel. Visual selection is shown while overlay open;
 * if user confirms while AI is thinking we schedule the change and apply it once safe.
 */
void handleSettingsOverlay(GameModel& model) {
    // Initialize visual selection when overlay opens
    if (settingsPendingSelection == -1) {
        settingsPendingSelection = static_cast<int>(currentDifficulty);
    }

    // --- Mouse click handling ---
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Cycle visual selection
        if (isMousePointerOverDifficultyButton()) {
            switch (settingsPendingSelection) {
            case (int)AIDifficulty::AI_EASY:    settingsPendingSelection = (int)AIDifficulty::AI_NORMAL; break;
            case (int)AIDifficulty::AI_NORMAL:  settingsPendingSelection = (int)AIDifficulty::AI_HARD;   break;
            case (int)AIDifficulty::AI_HARD:    settingsPendingSelection = (int)AIDifficulty::AI_EXTREME; break;
            case (int)AIDifficulty::AI_EXTREME: settingsPendingSelection = (int)AIDifficulty::AI_EASY;   break;
            default: settingsPendingSelection = (int)AIDifficulty::AI_NORMAL; break;
            }
        }
        // Confirm selection: apply immediately if safe, otherwise schedule
        else if (isMousePointerOverConfirmSettingsButton()) {
            if (settingsPendingSelection != -1) {
                AIDifficulty desired = static_cast<AIDifficulty>(settingsPendingSelection);
                if (aiThreadRunning) {
                    // schedule for when AI finishes
                    scheduledDifficulty = static_cast<int>(desired);
                    std::cout << "[Settings] AI is thinking; scheduling difficulty change to "
                        << AIFactory::getDifficultyName(desired) << std::endl;
                }
                else {
                    changeAIDifficulty(desired);
                }
            }
            // Close overlay and reset visual selection
            showSettingsOverlay = false;
            settingsPendingSelection = -1;
        }
        // Back to main menu from overlay
        else if (isMousePointerOverMainMenuButton()) {
            std::cout << "[Settings] Returning to main menu\n";
            showSettingsOverlay = false;
            cancelAIIfRunning(model);
            currentState = STATE_MAIN_MENU;
            initModel(model);
            settingsPendingSelection = -1;
        }
        // Close overlay without applying
        else if (isMousePointerOverCloseSettingsButton()) {
            std::cout << "[Settings] Closing settings\n";
            showSettingsOverlay = false;
            settingsPendingSelection = -1;
        }
    }

    // --- Slider dragging handling (unchanged) ---
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && isMousePointerOverNodeLimitSlider()) {
        Vector2 mousePos = GetMousePosition();
        Vector2 sliderPos = { SETTINGS_OVERLAY_X + SETTINGS_OVERLAY_WIDTH / 2.0f,
                              SETTINGS_NODE_LIMIT_Y };

        currentNodeLimit = getSliderValue(mousePos, sliderPos, SLIDER_WIDTH,
            NODE_LIMIT_MIN, NODE_LIMIT_MAX);

        // TODO: If AI instance supports node limit, apply it to currentAI here.
        // e.g. if (currentAI) currentAI->setNodeLimit(currentNodeLimit);
    }

    // If AI is not thinking and there is a scheduled difficulty, apply it now.
    if (!aiThreadRunning && scheduledDifficulty != -1) {
        applyScheduledDifficultyIfAny();
    }
}

/**
 * @brief Handles gameplay logic
 */
void handleGameplay(GameModel& model) {
    static double passMessageStartTime = 0;

    if (model.playedPass && !passMessageStartTime)
        passMessageStartTime = GetTime();
    else if (model.playedPass && passMessageStartTime) {
        double elapsed = GetTime() - passMessageStartTime;
        if (elapsed >= 1) {
            model.turnStartTime = GetTime();
            std::cout << "[Main] Pass message cleared, resuming as "
                << (model.currentPlayer == PLAYER_BLACK ? "BLACK" : "WHITE") << std::endl;
            model.playedPass = false;
            model.pauseTimers = false;
            passMessageStartTime = 0;
        }
        return;
    }

    if (aiEnabled && model.currentPlayer != model.humanPlayer) {
        checkAndApplyAIMove(model);
    }

    // Handle settings button
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (isMousePointerOverSettingsButton() && !showSettingsOverlay) {
            std::cout << "[Game] Opening settings overlay\n";
            showSettingsOverlay = true;
            // initialize visual selection when opening overlay
            settingsPendingSelection = static_cast<int>(currentDifficulty);
            return;
        }
    }

    // Game over screen
    if (model.gameOver && !showSettingsOverlay) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
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
        return;
    }

    // 1v1 mode (no AI)
    if (!aiEnabled && !showSettingsOverlay) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Move_t move = getMoveOnMousePointer();

            if (move != MOVE_NONE) {
                MoveList validMoves;
                getValidMoves(model, validMoves);

                auto it = std::find(validMoves.begin(), validMoves.end(), move);
                if (it != validMoves.end()) {
                    std::cout << "[Main] Player "
                        << (model.currentPlayer == PLAYER_BLACK ? "BLACK" : "WHITE")
                        << " plays move: " << (int)move << std::endl;
                    playMove(model, move);
                }
            }
        }
        return;
    }

    // 1 vs AI mode
    if (model.currentPlayer == model.humanPlayer && !showSettingsOverlay) {
        // Human player's turn
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
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
    else if(!showSettingsOverlay){
        // AI player's turn
        if (checkAndApplyAIMove(model)) {
            std::cout << "[Main] AI move processed, next player: "
                << (model.currentPlayer == model.humanPlayer ? "HUMAN" : "AI")
                << ", GameOver: " << model.gameOver << std::endl;
        }
        else if (!model.aiThinking && !model.gameOver) {
            std::cout << "[Main] AI turn detected, verifying valid moves..." << std::endl;

            MoveList aiMoves;
            getValidMoves(model, aiMoves);

            if (aiMoves.empty()) {
                std::cout << "[Main] WARNING: AI has no valid moves! Checking game over..." << std::endl;

                model.currentPlayer = getOpponent(model.currentPlayer);
                MoveList humanMoves;
                getValidMoves(model, humanMoves);

                if (humanMoves.empty()) {
                    std::cout << "[Main] Neither player can move - GAME OVER" << std::endl;
                    model.gameOver = true;
                }
                else {
                    std::cout << "[Main] AI passes turn to human" << std::endl;
                    model.playedPass = true;
                }
            }
            else {
                std::cout << "[Main] AI has " << aiMoves.size() << " valid moves, starting AI..." << std::endl;
                startAIThinking(model);
            }
        }
    }
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

    // Toggle fullscreen with Alt+Enter
    if ((IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) &&
        IsKeyPressed(KEY_ENTER)) {
        ToggleFullscreen();
    }

    // State machine
    switch (currentState) {
        case STATE_MAIN_MENU:
            handleMainMenu(model);
            break;

        case STATE_AI_SETTINGS_MENU:
            handleAISettingsMenu();
            break;

        case STATE_PLAYING:
        {
            handleGameplay(model);

            if (showSettingsOverlay) {
                handleSettingsOverlay(model);
            }

            std::string displayedDifficulty = (showSettingsOverlay && settingsPendingSelection != -1)
                ? getDifficultyString(static_cast<AIDifficulty>(settingsPendingSelection))
                : getDifficultyString(currentDifficulty);

            drawView(model, showSettingsOverlay, displayedDifficulty, currentNodeLimit, aiEnabled);
            break;
        }
    }
    return true;
}
