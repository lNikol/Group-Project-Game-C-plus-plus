#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

namespace RPG {

    class GameObject;

    class ISystem {
    public:
        virtual ~ISystem() = default;
        virtual void update(float dt, std::vector<std::unique_ptr<GameObject>>& entities) {}
        virtual void draw(sf::RenderWindow& window, std::vector<std::unique_ptr<GameObject>>& entities) {}
    };

}
