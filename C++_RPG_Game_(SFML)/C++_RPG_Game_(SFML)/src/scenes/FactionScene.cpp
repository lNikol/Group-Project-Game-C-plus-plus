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

    void FactionScene::handleEvents(const sf::Event& event) {
        BaseGameScene::handleEvents(event);
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        }

        // Clicking buildings on map
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            // sf::Mouse::Left ...
        }
    }

    void FactionScene::draw(sf::RenderWindow& window, const AssetManager& am) {
        BaseGameScene::draw(window, am);

        // Overlay drawing
        // window.draw(factionNameText);
    }
}