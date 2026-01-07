#include "core/GameManager.h"
#include <ctime>
int main() {
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	RPG::GameManager game;
	game.run();

	return 0;
}