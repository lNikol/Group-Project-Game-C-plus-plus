#pragma once
#include "BaseGameScene.h"

namespace RPG {

    class FactionScene : public BaseGameScene {
    public:
        FactionScene(AssetManager& am, SpritesheetManager& sm,
            std::shared_ptr<WorldMap> wm, std::shared_ptr<Player> p);

        void handleEvents(const sf::Event& event) override;

        void draw(sf::RenderWindow& window) override;
    };
}