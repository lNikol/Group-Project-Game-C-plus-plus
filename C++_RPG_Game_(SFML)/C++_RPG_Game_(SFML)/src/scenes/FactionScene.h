#pragma once
#include "BaseGameScene.h"

namespace RPG {

    /**
     * @brief Scene dedicated to faction management and diplomatic relations.
     */
    class FactionScene : public BaseGameScene {
    public:
        /**
         * @brief Constructs the FactionScene with required managers.
         * @param ctrl Scene controller.
         * @param am Asset manager for fonts/textures.
         * @param sm Spritesheet manager for faction icons.
         * @param wm The world map data.
         * @param p The player instance.
         */
        FactionScene(ISceneController& ctrl, std::shared_ptr<WorldMap> wm);

        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;

        void draw(sf::RenderWindow& window) override;
    };
}