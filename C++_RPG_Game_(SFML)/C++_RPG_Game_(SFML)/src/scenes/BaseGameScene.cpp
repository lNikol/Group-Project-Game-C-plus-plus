    #include "BaseGameScene.h"
    #include "core/Constants.h"

    namespace RPG {

        BaseGameScene::BaseGameScene(
            ISceneController& ctrl, std::shared_ptr<WorldMap> wm)
            : sceneController(ctrl), worldMap(wm), viewVisibility(0.5f)
        {
            camera.setSize(sf::Vector2f(static_cast<float>(Window::WIDTH), static_cast<float>(Window::HEIGHT)));
        }

        void BaseGameScene::update(float dt) {
            camera.setCenter(worldMap->getPlayer()->getPosition());
            
            for (auto& obj : worldMap->getGameObjects()) {
                obj->update(dt);
            }
        }

        void BaseGameScene::draw(sf::RenderWindow& window, const AssetManager& am) {
            window.setView(camera);

            if (worldMap) {
                worldRenderer.draw(window, *worldMap, am, viewVisibility);
            }

        }

        bool BaseGameScene::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
            if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::E) {
                    std::cout << "[" << sceneName << "]" << " Key 'E' was pressed: " << std::endl;
                    this->checkInteraction();
                }

                if (keyPressed->scancode == sf::Keyboard::Scancode::I) {
                    std::cout << "[" << sceneName << "]" << " Key 'I' was pressed: " << std::endl;
                    isInventoryOpen = !isInventoryOpen;
                }
            }
            return false;
        }

        void BaseGameScene::checkInteraction() {
            if (!worldMap || !worldMap->getPlayer()) return;

            for (auto& go : worldMap->getGameObjects()) {
                if (auto* interaction = go->getComponent<InteractionComponent>()) {
                    sf::Vector2f diff = worldMap->getPlayer()->getPosition() - go->getPosition();
                    float distSq = diff.x * diff.x + diff.y * diff.y;
                    float radius = interaction->getInteractionRadius();

                    if (distSq <= radius * radius) {
                        handleInteraction(go.get());
                        break;
                    }
                }
            }
        }

        void BaseGameScene::handleInteraction(GameObject* go) {
            std::cout << "Interacting with GameObject: " << go->getPrefabId() << std::endl;

            if (auto* npcComponent = go->getComponent<NpcComponent>()) {
                if (npcComponent->isFactionLeader()) {
                    std::cout << "[" << sceneName << "] This NPC is a Faction Leader. Switching map..." << std::endl;

                    sceneController.changeScene(npcComponent->getTargetFaction());
                }
            }
        }
    }