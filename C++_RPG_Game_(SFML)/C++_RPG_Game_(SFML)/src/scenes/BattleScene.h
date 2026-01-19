#pragma once
#include "BaseGameScene.h"

namespace RPG {
    class BattleScene : public Scene {
    private:
        ISceneController& controller;
        std::shared_ptr<WorldMap> battleMap;
        // your objects: std::vector<Enemy>, TurnManager itp.

    public:
        BattleScene(ISceneController& ctrl, std::shared_ptr<WorldMap> map)
            : controller(ctrl), battleMap(map) {
            // Battle UI
        }

        void update(float dt, const sf::RenderWindow& window) override {
            // if (turn == PLAYER) { ... }
        }

        void handleEvents(const sf::Event& event) override {
            // Mouse handler (choosing skills/targets)
        }

        void draw(sf::RenderWindow& window) override {
            // Draw map and battlefield units
        }
    };
}