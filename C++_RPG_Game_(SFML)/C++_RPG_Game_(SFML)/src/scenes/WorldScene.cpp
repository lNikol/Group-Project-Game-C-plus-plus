#include "WorldScene.h"
#include <iostream>

namespace RPG {

    WorldScene::WorldScene(ISceneController& ctrl, std::shared_ptr<WorldMap> wm)
        : BaseGameScene(ctrl, wm) 
    {
        sceneName = "WorldScene";
        viewVisibility = 0.8f;
        RPG::AudioManager::getInstance().stopMusic();
        RPG::AudioManager::getInstance().playMusic("assets/music/world-music.mp3");
        RPG::AudioManager::getInstance().loadFootsteps("assets/sounds/grass-sound.wav");
    }

    bool WorldScene::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        BaseGameScene::handleEvent(window, event);

        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->scancode == sf::Keyboard::Scancode::F) {
            }
        }
        return false;
    }
}