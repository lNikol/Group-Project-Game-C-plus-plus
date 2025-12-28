#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "core/Constants.h"
#include "core/SpritesheetManager.h"

namespace RPG {

	class GameManager {

		bool running;
		sf::RenderWindow window;
		SpritesheetManager spritesheetManager;
		sf::RectangleShape rect;

		void update(float dt);
		void draw();
		void handleEvents();

	public:
		GameManager();

		void run();

	};

}

