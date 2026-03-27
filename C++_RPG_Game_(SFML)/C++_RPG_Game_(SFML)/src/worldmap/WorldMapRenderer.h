#pragma once
#include <SFML/Graphics.hpp>
#include "WorldMap.h"
#include "AssetManager.h"
#include "Player.h"
#include <functional>
#include <algorithm>

namespace RPG {

    struct RenderPacket {
        float sortY; // The Y coordinate of the "feet"
        std::function<void(sf::RenderWindow&)> drawFunc; // How to draw it

        // Sort operator: Smaller Y (top of screen) draws first
        bool operator<(const RenderPacket& other) const {
            return sortY < other.sortY;
        }
    };

    /** @brief Handles the visual representation of the WorldMap */
    class WorldMapRenderer {
    private:
        sf::RectangleShape tileRect;

    public:
        WorldMapRenderer();

        /** * @brief Draws the visible portion of the map
         * @param sm Texture storage
         */
        void draw(
            sf::RenderWindow& window, 
            const WorldMap& worldMap,  
            const Player& player,
            float percentView = .9f
        );
    };
}