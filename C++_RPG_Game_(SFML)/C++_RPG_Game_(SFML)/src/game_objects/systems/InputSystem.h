#pragma once
#include "game_objects/systems/ISystem.h"
#include "game_objects/components/PlayerInputComponent.h"
#include "game_objects/components/MovementComponent.h"
#include "game_objects/GameObject.h"
#include "scenes/ISceneController.h"
#include <cmath>

namespace RPG {

    class InputSystem : public ISystem {
        const ISceneController& m_ctrl;
    public:
        InputSystem(const ISceneController& ctrl) : m_ctrl(ctrl) {}

        void update(float dt, std::vector<std::unique_ptr<GameObject>>& entities) override {
            if (!m_ctrl.hasWindowFocus()) {
                for (auto& entity : entities) {
                    if (!entity->hasComponent<PlayerInputComponent>()) continue;
                    if (auto* mov = entity->getComponent<MovementComponent>()) {
                        mov->setDirection({0.f, 0.f});
                    }
                }
                return;
            }

            for (auto& entity : entities) {
                if (!entity->hasComponent<PlayerInputComponent>()) continue;
                auto* mov = entity->getComponent<MovementComponent>();
                if (!mov) continue;

                sf::Vector2f dir(0.f, 0.f);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    dir.y -= 1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  dir.y += 1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  dir.x -= 1.f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) dir.x += 1.f;

                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len > 0.f) dir /= len;
                mov->setDirection(dir);
            }
        }
    };

}
