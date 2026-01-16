#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>
#include <memory>
#include "core/Constants.h"
#include "core/SpritesheetManager.h"
#include "worldmap/AssetManager.h"
#include "worldmap/WorldMap.h"
#include "worldmap/WorldMapRenderer.h"
#include "worldmap/Player.h"
#include "scenes/Scene.h"

namespace RPG {
	
	enum class FactionID { MainWorld, WhiteOrder, DarkOrder, BlueVanguard, GreenNature };

	class GameManager {

		bool running;
		sf::RenderWindow window;

		SpritesheetManager spritesheetManager;
		AssetManager assetManager;

		// shared info between scenes
		std::shared_ptr<Player> player;
		std::map<FactionID, std::shared_ptr<WorldMap>> allMaps;
		
		std::unique_ptr<Scene> currentScene;


		void handleEvents();
		void update(float dt);
		void draw();

		void initGameData();

	public:
		GameManager();
		void run();
		void changeScene(FactionID targetFaction);
	};

}

