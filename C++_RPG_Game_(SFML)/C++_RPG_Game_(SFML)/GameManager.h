#pragma once
#include <SFML/Graphics.hpp>

namespace RPG {

	class GameManager {

		bool running;
		sf::RenderWindow window;

	public:
		GameManager();

		void run();

	};

}

