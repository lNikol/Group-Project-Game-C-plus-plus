#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>
#include <memory>

#include "Constants.h"
#include "SpritesheetManager.h"
#include "worldmap/AssetManager.h"
#include "worldmap/WorldMap.h"
#include "worldmap/WorldMapRenderer.h"
#include "worldmap/Player.h"
#include "worldmap/enums.h"
#include "scenes/Scene.h"
#include "scenes/ISceneController.h"
#include "GUI/windows/BattleHud.h"

namespace RPG {
	
	/**
	 * @class GameManager
	 * @brief The central controller of the game.
	 * * Manages the main game loop, window, assets, and transitions between different game maps
	 * and scenes (e.g., World exploration, Faction bases).
	 */
	class GameManager : public ISceneController {

		bool running;
		sf::RenderWindow window;
		AssetManager assetManager;

		// shared info between scenes
		std::shared_ptr<Player> player;
		std::map<FactionID, std::shared_ptr<WorldMap>> allMaps;

		// UI & Data
		std::unique_ptr<BattleHUD> m_hud;
		const Spritesheet* m_iconSet;
		const sf::Font* m_globalFont;

		std::unique_ptr<Scene> currentScene;
		FactionID activeFaction;

		sf::Vector2f lastWorldPosition;

		void handleEvents();
		void update(float dt);
		void draw();

		/**
		 * @brief Initializes all map instances and shared game data.
		 * * Creates the Main World map and unique maps for each faction (White, Dark, etc.).
		 * Also initializes the shared Player object.
		 */
		void initGameData();

	public:
		GameManager();
		void run();

		/**
		 * @brief Transitions the game to a new scene based on the target faction.
		 * * This method handles memory management for the old scene and injects the correct
		 * WorldMap and Player data into the new scene.
		 * * @param targetFaction The ID of the faction map/world to load.
		 */
		void changeScene(FactionID targetFaction) override;
	};

}

