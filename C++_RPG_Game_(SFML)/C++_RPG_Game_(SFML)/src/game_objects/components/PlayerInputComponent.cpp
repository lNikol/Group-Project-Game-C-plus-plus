#include "PlayerInputComponent.h"
#include "game_objects/GameObject.h"
#include "MovementComponent.h"

namespace RPG {


    void PlayerInputComponent::init() {
        movementComponent = owner->getComponent<MovementComponent>();

        assert(movementComponent != nullptr
            && "PlayerInputComponent added to a GameObject missing a MovementComponent!");
    }

    void PlayerInputComponent::update(float dt) {
        if (!movementComponent) return;

        sf::Vector2f inputDirection(0.f, 0.f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            inputDirection.y -= 1.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            inputDirection.y += 1.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            inputDirection.x -= 1.0f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            inputDirection.x += 1.0f;

        if (inputDirection.x != 0.f || inputDirection.y != 0.f) {
            float length = std::sqrt((inputDirection.x * inputDirection.x) + (inputDirection.y * inputDirection.y));
            if (length > 0.f) {
                inputDirection /= length;
            }
        }

        movementComponent->setDirection(inputDirection);
	}

}