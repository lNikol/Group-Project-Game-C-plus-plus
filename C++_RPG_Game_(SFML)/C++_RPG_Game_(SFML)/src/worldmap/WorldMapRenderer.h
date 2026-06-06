#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include "WorldMap.h"
#include "AssetManager.h"

namespace RPG {

    class WorldMapRenderer {
        sf::RectangleShape tileRect;
    public:
        WorldMapRenderer();
        void setupView(sf::RenderWindow& window, const WorldMap& worldMap);
        void drawGround(sf::RenderWindow& window, const WorldMap& worldMap);
        void drawDebugHUD(sf::RenderWindow& window, const WorldMap& worldMap);
    };

}
