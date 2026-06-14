#include "scenes/BattleScene.h"
#include "combat/EncounterLoader.h"
#include <iostream>

namespace RPG {
    BattleScene::BattleScene(ISceneController& ctrl, const sf::RenderWindow& window, std::shared_ptr<Unit> player, const std::string& encounterFilePath)
        : m_controller(ctrl)
    {
        m_battleManager = std::make_unique<BattleManager>(player);
        auto encounterOpt = EncounterLoader::loadFromFile(encounterFilePath);

        // Stop old and play new music
        RPG::AudioManager::getInstance().stopMusic();
        RPG::AudioManager::getInstance().playMusic(
            "assets/music/battle-music.mp3"
        );
        if (encounterOpt.has_value()) {
            const EncounterData& data = encounterOpt.value();
            m_encounterName = data.name;
            m_battleManager->loadEncounter(data, window);
        }
        else {
            m_controller.changeScene(FactionID::MainWorld);
        }
    }

    bool BattleScene::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        if (m_battleManager) m_battleManager->handleEvent(window, event);
        return true;
    }

    void BattleScene::update(float dt, const sf::RenderWindow& window) {
        if (!m_battleManager) return;
        m_battleManager->update(dt);

        if (m_battleManager->getBattleResult() != BattleResult::Pending) {
            m_endTimer += dt;
            if (m_endTimer >= 3.0f) {
                if (m_battleManager->getBattleResult() == BattleResult::Victory) {
                    m_controller.changeScene(FactionID::MainWorld);
                }
                else {
                    m_controller.changeScene(FactionID::WhiteOrder);
                }
            }
        }
    }

    void BattleScene::draw(sf::RenderWindow& window) {
        if (m_battleManager) m_battleManager->render(window);
    }
}