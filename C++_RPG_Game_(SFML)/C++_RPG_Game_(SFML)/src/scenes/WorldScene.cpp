#include "WorldScene.h"
#include <iostream>

namespace RPG {

    WorldScene::WorldScene(ISceneController& ctrl, std::shared_ptr<WorldMap> wm)
        : BaseGameScene(ctrl, wm) 
    {
        sceneName = "WorldScene";
        viewVisibility = 0.8f;
    }

    bool WorldScene::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        BaseGameScene::handleEvent(window, event);
        return false;
    }
}