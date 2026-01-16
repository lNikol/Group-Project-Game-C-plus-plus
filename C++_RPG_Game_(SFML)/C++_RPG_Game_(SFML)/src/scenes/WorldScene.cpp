#include "WorldScene.h"
#include <iostream>

namespace RPG {

    WorldScene::WorldScene(AssetManager& am, SpritesheetManager& sm,
        std::shared_ptr<WorldMap> wm, std::shared_ptr<Player> p)
        : BaseGameScene(am, sm, wm, p) 
    {
        viewVisibility = 0.8f;
    }

    void WorldScene::handleEvents(const sf::Event& event) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {

            if (keyPressed->scancode == sf::Keyboard::Scancode::F) {
                std::cout << "[WorldScene] Signal: Switch to Faction Map" << std::endl;
            }

            if (keyPressed->scancode == sf::Keyboard::Scancode::E) {
                std::cout << "[WorldScene] Interaction key 'E' pressed" << std::endl;
            }
        }
    }
}