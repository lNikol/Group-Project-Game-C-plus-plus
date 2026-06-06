#pragma once
#include "game_objects/systems/ISystem.h"
#include "game_objects/GameObject.h"
#include <algorithm>

namespace RPG {

    class RenderSystem : public ISystem {
    public:
        void draw(sf::RenderWindow& window, std::vector<std::unique_ptr<GameObject>>& entities) override {
            std::vector<GameObject*> queue;
            queue.reserve(entities.size());
            for (auto& e : entities) queue.push_back(e.get());

            std::sort(queue.begin(), queue.end(), [](const GameObject* a, const GameObject* b) {
                if (a->getZIndex() != b->getZIndex()) return a->getZIndex() < b->getZIndex();
                return a->getPosition().y < b->getPosition().y;
            });

            for (auto* obj : queue) obj->draw(window);
        }
    };

}
