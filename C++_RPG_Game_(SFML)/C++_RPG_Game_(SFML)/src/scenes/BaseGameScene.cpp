#include "BaseGameScene.h"

namespace RPG {

    BaseGameScene::BaseGameScene(ISceneController& ctrl, std::shared_ptr<WorldMap> wm)
        : sceneController(ctrl), worldMap(wm)
    {
        initSystems();
    }

    void BaseGameScene::initSystems() {
        systemManager.addSystem<InputSystem>();
        systemManager.addSystem<MovementSystem>(*worldMap);
        systemManager.addSystem<AnimationSystem>();
        systemManager.addSystem<RenderSystem>();
    }

    // ==============================
    // Update
    // ==============================

    void BaseGameScene::update(float dt) {
        if (!worldMap) return;
        systemManager.update(dt, worldMap->getGameObjects());
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
    }

    // ==============================
    // Events
    // ==============================

    bool BaseGameScene::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->scancode == sf::Keyboard::Scancode::E) {
                checkInteraction();
            }
            if (key->scancode == sf::Keyboard::Scancode::I) {
                isInventoryOpen = !isInventoryOpen;
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
    }

}
