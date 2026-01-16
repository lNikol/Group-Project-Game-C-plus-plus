#pragma once
#include <SFML/Graphics.hpp>

namespace RPG {

    class Scene {
    public:
        virtual ~Scene() = default;
        virtual void handleEvents(const sf::Event& event) = 0;
        virtual void update(float dt, const sf::RenderWindow& window) = 0;
        virtual void draw(sf::RenderWindow& window) = 0;
    };
}