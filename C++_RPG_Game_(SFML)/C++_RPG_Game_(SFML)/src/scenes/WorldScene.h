#pragma once
#include "BaseGameScene.h"
#include <memory>
#include "combat/AudioManager.h"

namespace RPG {

    /**
     * @brief The primary gameplay scene where the player navigates the world map.
     */
    class WorldScene : public BaseGameScene {
    public:
        /**
         * @brief Constructs the WorldScene.
         * @param ctrl Scene controller.
         * @param am Asset manager.
         * @param sm Spritesheet manager.
         * @param wm The world map data to be rendered.
         * @param p The player entity navigating the world.
         */
        WorldScene(ISceneController& ctrl, std::shared_ptr<WorldMap> wm);

        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;
    };

}