/**
 * @brief Reversi game
 * @author Marc S. Ressl
 * @modified: AI modularization refactor
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "model.h"
#include "view.h"
#include "controller.h"
#include "ai/ai_factory.h"  // NUEVO: Para gestión de AI

int main()
{
	GameModel model;

	// NUEVO: Inicializar sistema de AI ANTES del modelo
	// Por defecto usamos AI_HARD, pero puedes cambiarlo aquí
	initializeAI(AI_EXTREME);

	initModel(model);
	initView();

	while (updateView(model));

	freeView();

	return 0;
}