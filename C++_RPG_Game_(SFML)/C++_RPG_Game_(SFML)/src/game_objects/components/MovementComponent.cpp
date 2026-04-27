#include "MovementComponent.h"
#include "game_objects/GameObject.h"
#include "../../worldmap/WorldMap.h"

namespace RPG {

	MovementComponent::MovementComponent(WorldMap* worldmap, float speed)
		: speed(speed), direction(0.f, 0.f), worldmap(worldmap)
	{
	}

	void MovementComponent::update(float dt) {
        if (!owner) return;

        sf::Vector2f velocity = direction * speed * dt;
        if (velocity.x == 0.f && velocity.y == 0.f) return;

        auto* collider = owner->getComponent<ColliderComponent>();
        if (!collider) {
            owner->setPosition(owner->getPosition() + velocity);
            return;
        }

        sf::FloatRect futureHitbox = collider->getGlobalHitbox();
        sf::Vector2f finalMovement(0.f, 0.f);

        futureHitbox.position.x += velocity.x;
        if (!worldmap->checkCollision(futureHitbox, owner)) {
            finalMovement.x = velocity.x;
        }
        else {
            futureHitbox.position.x -= velocity.x;
        }

        futureHitbox.position.y += velocity.y;
        if (!worldmap->checkCollision(futureHitbox, owner)) {
            finalMovement.y = velocity.y;
        }

        owner->setPosition(owner->getPosition() + finalMovement);
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

    const WorldMap* MovementComponent::getWorldMap() const {
        return worldmap;
    }

    void MovementComponent::setWorldMap(WorldMap* worldmap) {
        this->worldmap = worldmap;
    }

}
