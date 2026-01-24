    #include "BaseGameScene.h"
    #include "core/Constants.h"

    namespace RPG {

        BaseGameScene::BaseGameScene(
            ISceneController& ctrl, std::shared_ptr<WorldMap> wm, std::shared_ptr<Player> p
        )
            : sceneController(ctrl), worldMap(wm), player(p), viewVisibility(1.0f)
        {
            camera.setSize(sf::Vector2f(static_cast<float>(Window::WIDTH), static_cast<float>(Window::HEIGHT)));
        }

        void BaseGameScene::update(float dt, const sf::RenderWindow& window) {
            if (player && worldMap) {
                player->update(dt, *worldMap, window);

                camera.setCenter(player->getPosition());
            }

            /*
            // Update all NPCs on the assigned map
            for (auto& npc : worldMap->getNPCs()) {
                npc->update(dt, *worldMap);
            }*/
        }

        void BaseGameScene::draw(sf::RenderWindow& window, const AssetManager& am) {
            window.setView(camera);

            if (worldMap) {
                worldRenderer.draw(window, *worldMap, am, viewVisibility);
            }

            for (auto& npc : worldMap->getNPCs()) {
                    npc->draw(window, am);
            }

            if (player) {
                player->draw(window, am);
            }
        }

        bool BaseGameScene::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
            if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::E) {
                    std::cout << "[" << sceneName << "]" << " Key 'E' was pressed: " << std::endl;
                    this->checkNPCInteraction();
                }

                if (keyPressed->scancode == sf::Keyboard::Scancode::I) {
                    std::cout << "[" << sceneName << "]" << " Key 'I' was pressed: " << std::endl;
                    isInventoryOpen = !isInventoryOpen;
                }
            }
            return false;
        }

        void BaseGameScene::checkNPCInteraction() {
            if (!worldMap || !player) return;

            for (auto& npc : worldMap->getNPCs()) {
                sf::Vector2f diff = player->getPosition() - npc->getPosition();
                float distSq = diff.x * diff.x + diff.y * diff.y;
                float radius = npc->getInteractionRadius();

                if (distSq <= radius * radius) {
                    handleInteraction(npc.get());
                    break;
                }
            }
        }

        void BaseGameScene::handleInteraction(NPC* npc) {
            std::cout << "Interacting with NPC: " << npc->getName() << std::endl;

            if (npc->getIsFactionLeader()) {
                std::cout << "[" << sceneName << "] This NPC is a Faction Leader. Switching map..." << std::endl;

                sceneController.changeScene(npc->getTargetFaction());
            }
        }
    }