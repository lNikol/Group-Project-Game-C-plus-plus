#pragma once
#include "scenes/Scene.h"
#include "scenes/ISceneController.h"
#include "worldmap/WorldMap.h"
#include "worldmap/AssetManager.h"
#include "combat/BattleManager.h"
#include <memory>

namespace RPG {

    class BattleScene : public Scene {
    public:
        BattleScene(ISceneController& ctrl,
                    const sf::RenderWindow& window,
                    std::shared_ptr<Unit> player,
                    std::shared_ptr<WorldMap> map);

        virtual ~BattleScene() = default;

        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;
        void update(float dt) override;
        void draw(sf::RenderWindow& window) override;

    private:
        ISceneController&              m_controller;
        std::unique_ptr<BattleManager> m_battleManager;
    };

}
