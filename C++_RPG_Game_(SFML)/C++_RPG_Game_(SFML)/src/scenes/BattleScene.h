#pragma once
#include "scenes/Scene.h"
#include "scenes/ISceneController.h"
#include "worldmap/WorldMap.h"
#include "worldmap/Player.h"
#include "worldmap/AssetManager.h"
#include "combat/BattleManager.h"
#include <memory>
#include "combat/AudioManager.h"

namespace RPG {

    /**
     * @brief The high-level Scene wrapper for the Combat Phase.
     * * Responsibilities:
     * 1. Owns the BattleManager (the actual combat logic).
     * 2. Bridges global resources (AssetManager, Player) into the combat system.
     * 3. Forwards Update/Draw/Event calls from the main loop to the BattleManager.
     */
    class BattleScene : public Scene {
    public:
        /**
         * @brief Constructs the Battle Scene.
         * @param ctrl Reference to the main game controller (for switching back to World later).
         * @param player The persistent player data (HP, Inventory) to use in the fight.
         * @param map The generic WorldMap data (can be used to generate obstacles).
         */
        BattleScene(ISceneController& ctrl,
            const sf::RenderWindow& window,
            std::shared_ptr<Unit> player,
            std::shared_ptr<WorldMap> map);

        virtual ~BattleScene() = default;

        // ==============================
        // Scene Lifecycle
        // ==============================

        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;
        void update(float dt) override;
        void draw(sf::RenderWindow& window) override;

    private:
        ISceneController& m_controller;

        // The core logic controller for the battle
        std::unique_ptr<BattleManager> m_battleManager;
    };
}