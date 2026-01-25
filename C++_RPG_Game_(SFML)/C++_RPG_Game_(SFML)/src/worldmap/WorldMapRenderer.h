#pragma once
#include <SFML/Graphics.hpp>
#include "WorldMap.h"
#include "AssetManager.h"

namespace RPG {

    /** @brief Handles the visual representation of the WorldMap */
    class WorldMapRenderer {
    private:
        sf::RectangleShape tileRect;

    public:
        WorldMapRenderer();

        /** * @brief Draws the visible portion of the map
         * @param assetManager Definitions for structure properties
         * @param sm Texture storage
         */
        void draw(sf::RenderWindow& window, const WorldMap& worldMap, const AssetManager& assetManager, float percentView = 0.9f);
    };
}