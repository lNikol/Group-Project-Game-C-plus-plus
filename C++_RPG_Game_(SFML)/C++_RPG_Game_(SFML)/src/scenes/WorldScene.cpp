#include "WorldScene.h"
#include <iostream>

namespace RPG {

    WorldScene::WorldScene(ISceneController& ctrl, std::shared_ptr<WorldMap> wm, std::shared_ptr<Player> p)
        : BaseGameScene(ctrl, wm, p) 
    {
        sceneName = "WorldScene";
        viewVisibility = 0.8f;
    }

    void WorldScene::handleEvents(const sf::Event& event) {
        BaseGameScene::handleEvents(event);

        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->scancode == sf::Keyboard::Scancode::F) {
            }
        }
    }
}