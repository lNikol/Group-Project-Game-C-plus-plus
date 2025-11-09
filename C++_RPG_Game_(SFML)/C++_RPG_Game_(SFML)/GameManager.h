#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Constants.h"

namespace RPG {

	class GameManager {

		bool running;
		sf::RenderWindow window;

		void update(float dt);
		void draw();
		void handleEvents();

	public:
		GameManager();

		void run();

	};

}

