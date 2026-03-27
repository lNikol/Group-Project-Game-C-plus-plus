#include "FactionScene.h"

namespace RPG {

    FactionScene::FactionScene(
        ISceneController& ctrl, std::shared_ptr<WorldMap> wm, std::shared_ptr<Player> p
    )
        : BaseGameScene(ctrl, wm, p)
    {
        sceneName = "FactionScene";
        viewVisibility = 1.0f; 
    }

    bool FactionScene::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        BaseGameScene::handleEvent(window, event);
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        }

        // Clicking buildings on map
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            // sf::Mouse::Left ...
        }
        return false;
    }

    void FactionScene::draw(sf::RenderWindow& window) {
        BaseGameScene::draw(window);

        // Overlay drawing
        // window.draw(factionNameText);
    }
}