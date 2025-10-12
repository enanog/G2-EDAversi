/**
 * @brief Reversi game
 * @author Marc S. Ressl
 * @modified: AI modularization refactor
 *
 * @copyright Copyright (c) 2023-2024
 */

#include "model.h"
#include "view/view.h"
#include "controller.h"
#include "ai/ai_factory.h"

int main()
{
	GameModel model;
	initializeAI(AI_NORMAL);

	initModel(model);
	initView();

	while (updateView(model));

	freeView();

	return 0;
}