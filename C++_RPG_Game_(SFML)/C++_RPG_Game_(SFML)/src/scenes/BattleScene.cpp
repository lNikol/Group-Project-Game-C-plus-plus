#include "scenes/BattleScene.h"

namespace RPG {

    BattleScene::BattleScene(ISceneController& ctrl,
        const sf::RenderWindow& window,
        const AssetManager& assetManager,
        std::shared_ptr<Unit> player,
        std::shared_ptr<WorldMap> map)
        : m_controller(ctrl)
    {

        // 1. Initialize the Battle Manager
        // Pass the player so the Manager can add them to the grid
        m_battleManager = std::make_unique<BattleManager>(assetManager, player);

        // 2. Setup the initial state (Generate enemies, props)
        m_battleManager->initTestLevel(window);
    }

    bool BattleScene::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        if (!m_battleManager) return false;

        // Forward raw input to the manager
        m_battleManager->handleEvent(window, event);

        
        return false;
    }

    void BattleScene::update(float dt) {
        if (m_battleManager) {
            m_battleManager->update(dt);
        }
        if (m_battleManager->isBattleOver()) m_controller.changeScene(FactionID::MainWorld);
    }

    void BattleScene::draw(sf::RenderWindow& window, const AssetManager& am) {
        if (m_battleManager) {
            m_battleManager->render(window);
        }
    }
}