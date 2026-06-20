#include "BaseGameScene.h"
#include <iostream>
#include "core/EventBus.h"
#include "game_objects/components/components.h"
#include "core/QuestManager.h"

namespace RPG {

    BaseGameScene::BaseGameScene(ISceneController& ctrl, std::shared_ptr<WorldMap> wm)
        : sceneController(ctrl), worldMap(wm)
    {
        if (auto font = AssetManager::getInstance().getFont("PixelFont")) {
            m_journalWindow = std::make_unique<JournalWindow>(*font);
            m_journalWindow->setPosition({100.f, 100.f}); // Set an initial reasonable position
            m_journalWindow->hide();

            m_hudIconText = std::make_unique<sf::Text>(*font);
            m_hudIconText->setString("[J] Quest Book");
            m_hudIconText->setCharacterSize(20);
            m_hudIconText->setFillColor(sf::Color::White);

            m_hudIconBg.setSize({160.f, 40.f});
            m_hudIconBg.setFillColor(sf::Color(30, 30, 30, 200));
            m_hudIconBg.setOutlineColor(sf::Color(100, 100, 100));
            m_hudIconBg.setOutlineThickness(2.f);
        }
        m_lastActiveQuestCount = QuestManager::getInstance().getActiveQuests().size();
        initSystems();
    }

    void BaseGameScene::initSystems() {
        systemManager.addSystem<InputSystem>();
        systemManager.addSystem<MovementSystem>(*worldMap);
        systemManager.addSystem<AnimationSystem>();

        systemManager.addSystem<TriggerSystem>(
            *worldMap,
            std::vector<std::shared_ptr<ITriggerHandler>>{
            std::make_shared<GameTriggerHandler>(sceneController),
                std::make_shared<DialogTriggerHandler>()
        }
        );

        systemManager.addSystem<RenderSystem>();
    }

    void BaseGameScene::update(float dt, const sf::RenderWindow& window) {
        if (!worldMap) return;
        systemManager.update(dt, worldMap->getGameObjects());
        
        // Window sizing
        if (m_journalWindow) {
            sf::Vector2f winSize(window.getSize());
            sf::Vector2f targetSize(winSize.x * 0.8f, winSize.y * 0.8f);
            if (m_journalWindow->getSize() != targetSize) {
                m_journalWindow->setSize(targetSize);
                m_journalWindow->setPosition({winSize.x * 0.1f, winSize.y * 0.1f});
            }
            m_journalWindow->update(dt);
        }

        // HUD Icon Logic
        size_t currentQuests = QuestManager::getInstance().getActiveQuests().size();
        if (currentQuests > m_lastActiveQuestCount) {
            m_questIconJumpTimer = 1.5f; // Jump for 1.5s
            m_lastActiveQuestCount = currentQuests;
        } else if (currentQuests < m_lastActiveQuestCount) {
            m_lastActiveQuestCount = currentQuests; // Handle completions
        }

        if (m_questIconJumpTimer > 0.f) {
            m_questIconJumpTimer -= dt;
            if (m_questIconJumpTimer < 0.f) m_questIconJumpTimer = 0.f;
        }
    }


    // ==============================
    // Draw 
    // ==============================

    void BaseGameScene::draw(sf::RenderWindow& window) {
        if (!worldMap) return;

        // 1. Place camera centered on player
        worldRenderer.setupView(window, *worldMap);

        // 2. Draw ground (layer 0)
        worldRenderer.drawGround(window, *worldMap);

        // 3. RenderSystem sorts and draws all GameObjects (layer 1+)
        systemManager.draw(window, worldMap->getGameObjects());

        // 4. Debug HUD (player position, UI layer)
        worldRenderer.drawDebugHUD(window, *worldMap);

        // Save view and draw our custom HUD
        sf::View savedView = window.getView();
        window.setView(window.getDefaultView());

        // Position HUD Icon at Bottom Right
        sf::Vector2f winSize(window.getSize());
        float baseX = winSize.x - m_hudIconBg.getSize().x - 20.f;
        float baseY = winSize.y - m_hudIconBg.getSize().y - 20.f;

        // Apply jump animation
        float offsetY = 0.f;
        if (m_questIconJumpTimer > 0.f) {
            m_hudIconBg.setOutlineColor(sf::Color::Yellow);
            if (m_hudIconText) m_hudIconText->setFillColor(sf::Color::Yellow);
            offsetY = -std::abs(std::sin(m_questIconJumpTimer * 10.f)) * 15.f;
        } else {
            m_hudIconBg.setOutlineColor(sf::Color(100, 100, 100));
            if (m_hudIconText) m_hudIconText->setFillColor(sf::Color::White);
        }

        m_hudIconBg.setPosition({baseX, baseY + offsetY});
        if (m_hudIconText) m_hudIconText->setPosition({baseX + 10.f, baseY + offsetY + 8.f});

        window.draw(m_hudIconBg);
        if (m_hudIconText) window.draw(*m_hudIconText);

        // 5. Journal Window
        if (m_journalWindow && m_journalWindow->isVisible()) {
            window.draw(*m_journalWindow);
        }

        window.setView(savedView);
    }

    // ==============================
    // Events
    // ==============================

    bool BaseGameScene::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        sf::View savedView = window.getView();
        window.setView(window.getDefaultView());
        bool handled = false;
        
        if (m_journalWindow && m_journalWindow->handleEvent(window, event)) {
            handled = true;
        }
        
        window.setView(savedView);
        if (handled) return true;

        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->scancode == sf::Keyboard::Scancode::E) {
                checkInteraction();
                return true;
            }
            if (key->scancode == sf::Keyboard::Scancode::K) {
                std::cout << "[Debug] Simulating Rat Kill" << std::endl;
                RPG::EventBus::getInstance().publish(RPG::EnemyDefeatedEvent("rat"));
                return true;
            }
            if (key->scancode == sf::Keyboard::Scancode::I) {
                isInventoryOpen = !isInventoryOpen;
                return true;
            }
            if (key->scancode == sf::Keyboard::Scancode::J) {
                if (m_journalWindow) {
                    m_journalWindow->toggle();
                }
                return true;
            }
        }
        
        return false;
    }


    // ==============================
    // Interaction
    // ==============================

    void BaseGameScene::checkInteraction() {
        if (!worldMap || !worldMap->getPlayer()) return;

        for (auto& go : worldMap->getGameObjects()) {
            auto* interaction = go->getComponent<InteractionComponent>();
            if (!interaction) continue;

            sf::Vector2f diff = worldMap->getPlayer()->getPosition() - go->getPosition();
            float distSq = diff.x * diff.x + diff.y * diff.y;
            float radius = interaction->getInteractionRadius();

            if (distSq <= radius * radius) {
                handleInteraction(go.get());
                break;
            }
        }
    }

    void BaseGameScene::handleInteraction(GameObject* go) {
        if (auto* npc = go->getComponent<NpcComponent>()) {
            if (npc->isFactionLeader()) {
                sceneController.changeScene(npc->getTargetFaction());
            }
        }
        
        if (auto* questGiver = go->getComponent<QuestGiverComponent>()) {
            const std::string& completeId = questGiver->getQuestToComplete();
            const std::string& startId = questGiver->getQuestToStart();
            
            bool completed = false;
            if (!completeId.empty()) {
                if (QuestManager::getInstance().getQuestStatus(completeId) == QuestStatus::ReadyToTurnIn) {
                    completed = QuestManager::getInstance().completeQuest(completeId);
                }
            }
            if (!completed && !startId.empty()) {
                if (QuestManager::getInstance().getQuestStatus(startId) == QuestStatus::NotStarted) {
                    QuestManager::getInstance().startQuest(startId);
                }
            }
        }
    }

}
