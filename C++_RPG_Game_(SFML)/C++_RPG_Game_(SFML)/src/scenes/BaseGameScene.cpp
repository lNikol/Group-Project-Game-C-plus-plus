#include "BaseGameScene.h"
#include <iostream>
#include <cmath>
#include "core/EventBus.h"
#include "game_objects/components/components.h"
#include "core/QuestManager.h"
#include "core/DialogManager.h"

namespace RPG {

    BaseGameScene::BaseGameScene(ISceneController& ctrl, std::shared_ptr<WorldMap> wm)
        : sceneController(ctrl), worldMap(wm)
    {
        m_lastActiveQuestCount = QuestManager::getInstance().getActiveQuests().size();
        initSystems();
    }

    void BaseGameScene::initSystems() {
        systemManager.addSystem<InputSystem>(sceneController);
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
        
        // Window

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

        // Update distances for interaction indicators
        if (worldMap->getPlayer()) {
            sf::Vector2f playerPos = worldMap->getPlayer()->getPosition();
            for (auto& go : worldMap->getGameObjects()) {
                if (auto* interaction = go->getComponent<InteractionComponent>()) {
                    sf::Vector2f diff = playerPos - go->getPosition();
                    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
                    interaction->setDistanceToPlayer(dist);
                }
            }
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

        // 5. Journal

        window.setView(savedView);
    }

    // ==============================
    // Events
    // ==============================

    bool BaseGameScene::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        sf::View savedView = window.getView();
        window.setView(window.getDefaultView());
        bool handled = false;
        
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
        if (auto* dialogComp = go->getComponent<DialogComponent>()) {
            DialogManager::getInstance().startDialog(dialogComp->dialogId);
            return;
        }

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
