#pragma once
#include <vector>
#include <memory>
#include "game_objects/systems/ISystem.h"
#include "game_objects/GameObject.h"

namespace RPG {

    class SystemManager {
        std::vector<std::unique_ptr<ISystem>> systems;
    public:
        template<class T, class... Args>
        T* addSystem(Args&&... args) {
            auto sys = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = sys.get();
            systems.push_back(std::move(sys));
            return ptr;
        }

        void update(float dt, std::vector<std::unique_ptr<GameObject>>& entities) {
            for (auto& sys : systems) sys->update(dt, entities);
        }

        void draw(sf::RenderWindow& window, std::vector<std::unique_ptr<GameObject>>& entities) {
            for (auto& sys : systems) sys->draw(window, entities);
        }
    };

}
