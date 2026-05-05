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
#include "animation/AnimationManager.h"
#include "animation/Animation.h"
#include "animation/AnimationBuilder.h"
#include "combat/Unit.h"


#include "game_objects/GameObjectFactory.h"

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
		AnimationManager animationManager;

		// shared info between scenes
		std::map<FactionID, std::shared_ptr<WorldMap>> allMaps;
		std::shared_ptr<Unit> playerUnit;
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

		/**
		 * @brief Internal helper to manage the map lifecycle (Cache/Disk/Generation).
		 * @param id The FactionID/MapID to load.
		 * @return A shared pointer to the map instance.
		 */
		std::shared_ptr<WorldMap> getOrLoadMap(FactionID id);
		sf::Vector2u getMapSize(FactionID id);

	public:
		GameManager();
		void run();
		 /**
		  * @brief Handles transitions between different game areas.
		  * @details Now uses getOrLoadMap to ensure the target map is in memory
		  * (loaded from disk or generated) before switching the scene.
		  * @param targetFaction The ID of the faction map/world to load.
		  */
		void changeScene(FactionID targetFaction) override;

		~GameManager();
	};

}

