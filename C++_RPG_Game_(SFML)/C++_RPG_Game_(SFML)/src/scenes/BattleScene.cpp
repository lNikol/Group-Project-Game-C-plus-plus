#include "BattleScene.h"

namespace RPG {

    BattleScene::BattleScene(ISceneController& ctrl, std::shared_ptr<WorldMap> map)
        : controller(ctrl), battleMap(map) {
        // Battle UI
    }

    void BattleScene::update(float dt, const sf::RenderWindow& window) {
        // if (turn == PLAYER) { ... }
    }

    void BattleScene::handleEvents(const sf::Event& event) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {

        }
    }

    void BattleScene::draw(sf::RenderWindow& window) {
        // Draw map and battlefield units
    }
}