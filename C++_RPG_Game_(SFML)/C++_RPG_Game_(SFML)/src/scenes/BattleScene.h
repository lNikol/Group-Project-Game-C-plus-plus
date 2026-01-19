#pragma once
#include "BaseGameScene.h"

namespace RPG {

    /**
     * @brief Handles the turn-based combat gameplay.
     * * This scene manages combat actors, turn sequencing, and the tactical
     * user interface during a fight. It uses the battle map as its background.
     */

    class BattleScene : public Scene {
    private:
        ISceneController& controller;
        std::shared_ptr<WorldMap> battleMap;
        // your objects: std::vector<Enemy>, TurnManager itp.

    public:
        /**
         * @brief Constructs the BattleScene.
         * @param ctrl Reference to the scene controller for state transitions.
         * @param map Shared pointer to the map where the battle takes place.
         */
        BattleScene(ISceneController& ctrl, std::shared_ptr<WorldMap> map);

        void update(float dt, const sf::RenderWindow& window) override;

        void handleEvents(const sf::Event& event) override;

        void draw(sf::RenderWindow& window) override;
    };
}