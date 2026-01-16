#pragma once
#include "BaseGameScene.h"
#include <memory>

namespace RPG {

    class WorldScene : public BaseGameScene {
    public:
        WorldScene(AssetManager& am, SpritesheetManager& sm,
            std::shared_ptr<WorldMap> wm, std::shared_ptr<Player> p);

        void handleEvents(const sf::Event& event) override;
    };

}