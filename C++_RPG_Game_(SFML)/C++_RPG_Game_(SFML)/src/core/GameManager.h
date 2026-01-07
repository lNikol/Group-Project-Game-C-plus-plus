#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "core/Constants.h"
#include "core/SpritesheetManager.h"
#include "./worldmap/AssetManager.h"
#include "./worldmap/WorldMap.h"
#include "./worldmap/WorldMapRenderer.h"
#include "./worldmap/Player.h"

namespace RPG {

	class GameManager {

		bool running;
		sf::RenderWindow window;
		sf::View camera;

		SpritesheetManager spritesheetManager;
		AssetManager assetManager;
		WorldMapRenderer worldRenderer;

		std::unique_ptr<WorldMap> worldMap;
		std::unique_ptr<Player> player;


		void handleEvents();
		void update(float dt);
		void draw();

	public:
		GameManager();
		void run();

	};

}

