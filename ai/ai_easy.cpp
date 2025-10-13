/**
 * @brief Easy AI implementation
 * @author Marc S. Ressl
 * @modifiers:
 *          Agustin Valenzuela,
 *          Alex Petersen,
 *          Dylan Frigerio,
 *          Enzo Fernandez Rosas
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "ai_easy.h"
#include <chrono>
#include <iostream>

AIEasy::AIEasy() {
    // Seed RNG with current time for varied gameplay
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    rng.seed(static_cast<unsigned int>(seed));

    std::cout << "[AIEasy] Initialized with seed: " << seed << std::endl;
}

Move_t AIEasy::getBestMove(GameModel& model) {
    // Get all valid moves
    MoveList validMoves;
    getValidMoves(model, validMoves);

    if (validMoves.empty()) {
        return MOVE_NONE;
    }

    // Select random move from valid options
    std::uniform_int_distribution<size_t> dist(0, validMoves.size() - 1);
    size_t randomIndex = dist(rng);

    Move_t selectedMove = validMoves[randomIndex];

    return selectedMove;
}