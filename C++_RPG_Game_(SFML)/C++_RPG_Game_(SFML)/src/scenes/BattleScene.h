#pragma once
#include "scenes/Scene.h"
#include "scenes/ISceneController.h"
#include "worldmap/WorldMap.h"
#include "worldmap/AssetManager.h"
#include "combat/BattleManager.h"
#include <memory>
#include "combat/AudioManager.h"

namespace RPG {

    class BattleScene : public Scene {
    public:
        /**
         * @brief Constructs the Battle Scene.
         * @param ctrl Reference to the main game controller (for switching back to World later).
         * @param player The persistent player data (HP, Inventory) to use in the fight.
         * @param map The generic WorldMap data (can be used to generate obstacles).
         */
        BattleScene(ISceneController& ctrl, const sf::RenderWindow& window, std::shared_ptr<Unit> player, const std::string& encounterFilePath);

        virtual ~BattleScene() = default;

        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;
        void update(float dt, const sf::RenderWindow& window) override;
        void draw(sf::RenderWindow& window) override;

        bool isBattleScene() const override;


    private:
        ISceneController&              m_controller;
        std::unique_ptr<BattleManager> m_battleManager;
        std::string m_encounterName;
        float m_endTimer = 0.0f;
    };

}
