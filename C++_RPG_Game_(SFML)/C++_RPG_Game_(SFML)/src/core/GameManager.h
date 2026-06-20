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
#include "worldmap/enums.h"
#include "scenes/Scene.h"
#include "scenes/ISceneController.h"
#include "GUI/windows/SystemHUD.h"
#include "combat/Unit.h"
#include "game_objects/GameObjectFactory.h"
#include "game_objects/systems/DebugSystem.h"

namespace RPG {

    class GameManager : public ISceneController {
        bool running;
        sf::RenderWindow window;

        std::map<FactionID, std::shared_ptr<WorldMap>> allMaps;
        std::shared_ptr<Unit> playerUnit;

        std::unique_ptr<SystemHUD> m_systemHud;
        const Spritesheet* m_iconSet = nullptr;
        const sf::Font* m_globalFont = nullptr;

        std::unique_ptr<Scene> currentScene;
        FactionID activeFaction;
        sf::Vector2f lastWorldPosition;

        DebugSystem debugSystem;

        void handleEvents();
        void update(float dt);
        void draw();
        void initGameData();

        std::shared_ptr<WorldMap> getOrLoadMap(FactionID id);
        sf::Vector2u getMapSize(FactionID id);

    public:
        GameManager();
        void run();
        void changeScene(FactionID targetFaction) override;
        ~GameManager();
    };

}
