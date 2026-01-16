#include "BaseGameScene.h"
#include "core/Constants.h"

namespace RPG {

    BaseGameScene::BaseGameScene(AssetManager& am, SpritesheetManager& sm,
        std::shared_ptr<WorldMap> wm, std::shared_ptr<Player> p)
        : assetManager(am), spritesheetManager(sm), worldMap(wm), player(p), viewVisibility(1.0f)
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

    void BaseGameScene::draw(sf::RenderWindow& window) {
        window.setView(camera);

        if (worldMap) {
            worldRenderer.draw(window, *worldMap, assetManager, spritesheetManager, viewVisibility);
        }

        /*for (auto& npc : worldMap->getNPCs()) {
                npc->draw(window, spritesheetManager);
        }*/

        if (player) {
            player->draw(window, spritesheetManager);
        }
    }
}