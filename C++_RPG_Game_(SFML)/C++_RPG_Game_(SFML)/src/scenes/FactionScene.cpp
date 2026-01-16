#include "FactionScene.h"

namespace RPG {

    FactionScene::FactionScene(AssetManager& am, SpritesheetManager& sm,
        std::shared_ptr<WorldMap> wm, std::shared_ptr<Player> p)
        : BaseGameScene(am, sm, wm, p)
    {
        viewVisibility = 1.0f; 
    }

    void FactionScene::handleEvents(const sf::Event& event) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->scancode == sf::Keyboard::Scancode::E) {
                // Logic with Faction's NPC
            }
        }

        // Clicking buildings on map
        if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            // sf::Mouse::Left ...
        }
    }

    void FactionScene::draw(sf::RenderWindow& window) {
        BaseGameScene::draw(window);

        // Overlay drawing
        // window.draw(factionNameText);
    }
}