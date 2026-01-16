#pragma once
#include <memory>
#include <vector>
#include "Scene.h"
#include "worldmap/WorldMap.h"
#include "worldmap/WorldMapRenderer.h"
#include "worldmap/Player.h"
#include "worldmap/AssetManager.h"
#include "core/SpritesheetManager.h"


namespace RPG {

    class BaseGameScene : public Scene {
    protected:
        bool isInventoryOpen = false;
        // std::unique_ptr<GameUI> gameUI; // Or smth like that

        AssetManager& assetManager;
        SpritesheetManager& spritesheetManager;

        std::shared_ptr<WorldMap> worldMap;
        std::shared_ptr<Player> player;
        

        WorldMapRenderer worldRenderer;
        sf::View camera;
        float viewVisibility;

    public:
        BaseGameScene(AssetManager& am, SpritesheetManager& sm,
            std::shared_ptr<WorldMap> wm, std::shared_ptr<Player> p);

        virtual ~BaseGameScene() = default;

        void update(float dt, const sf::RenderWindow& window) override;
        void draw(sf::RenderWindow& window) override;

        virtual void handleEvents(const sf::Event& event) override = 0;
    };
}