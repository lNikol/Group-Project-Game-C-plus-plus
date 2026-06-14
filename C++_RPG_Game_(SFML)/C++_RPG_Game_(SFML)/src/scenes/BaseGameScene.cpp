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

        systemManager.addSystem<TriggerSystem>(
            *worldMap,
            std::vector<std::shared_ptr<ITriggerHandler>>{
            std::make_shared<GameTriggerHandler>(sceneController),   
            std::make_shared<DialogTriggerHandler>()
        }
        );

        void BaseGameScene::update(float dt, const sf::RenderWindow& window) {
            camera.setCenter(worldMap->getPlayer()->getPosition());
            
            for (auto& obj : worldMap->getGameObjects()) {
                obj->update(dt);
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
