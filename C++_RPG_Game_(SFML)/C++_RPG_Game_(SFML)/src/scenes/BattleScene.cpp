#include "BattleScene.h"

namespace RPG {

    BattleScene::BattleScene(ISceneController& ctrl, std::shared_ptr<WorldMap> map)
        : controller(ctrl), battleMap(map) {
        // Battle UI
    }

    void BattleScene::update(float dt, const sf::RenderWindow& window) override {
        // if (turn == PLAYER) { ... }
    }

    void BattleScene::handleEvents(const sf::Event& event) override {
        // Mouse handler (choosing skills/targets)
    }

    void BattleScene::draw(sf::RenderWindow& window) override {
        // Draw map and battlefield units
    }
}