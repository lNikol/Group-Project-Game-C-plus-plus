#include "MovementComponent.h"
#include "game_objects/GameObject.h"

namespace RPG {

	MovementComponent::MovementComponent(float speed)
		: speed(speed), direction(0.f, 0.f)
	{
	}

	void MovementComponent::update(float dt) {
		if (!owner) return;

		if (direction.x != 0.f || direction.y != 0.f) {
			sf::Vector2f velocity = direction * speed * dt;
			owner->setPosition(owner->getPosition() + velocity);
		}
	}

	void MovementComponent::setDirection(sf::Vector2f direction) {
		this->direction = direction;
	}

	void MovementComponent::setSpeed(float speed) {
		this->speed = speed;
	}

	sf::Vector2f MovementComponent::getVelocity() const {
		return direction * speed;
	}

}
