#include "CollidingEntity.h"

namespace RPG {

	CollidingEntity::CollidingEntity()
		: hitbox({ -10.f, 0.f }, { 20.f, 10.f })
	{}

	CollidingEntity::~CollidingEntity() {}

	void CollidingEntity::setHitbox(float width, float height, float offsetY) {
		sf::Vector2f pos = { -width / 2.f, offsetY };
		sf::Vector2f size = { width, height };
		hitbox = sf::FloatRect(pos, size);
	}

	void CollidingEntity::moveWithCollision(
		sf::Vector2f velocity, 
		const WorldMap& map
	) {
		if (velocity.x == 0 && velocity.y == 0) return;

		sf::Vector2f currentPos = getPosition();
		
		// Check x
		float nextX = currentPos.x + velocity.x;
		float globalLeft = nextX + hitbox.position.x;
		float globalRight = globalLeft + hitbox.size.x;
		float globalTop = currentPos.y + hitbox.position.y;
		float globalBottom = globalTop + hitbox.size.y;

		bool collisionX = false;

		if (velocity.x > 0) {
			if (map.isBlockingAtPixel(globalRight, globalTop) ||
				map.isBlockingAtPixel(globalRight, globalBottom)) {
				collisionX = true;
			}
		}
		else if (velocity.x < 0) {
			if (map.isBlockingAtPixel(globalLeft, globalTop) ||
				map.isBlockingAtPixel(globalLeft, globalBottom)) {
				collisionX = true;
			}
		}

		if (!collisionX) {
			move({ velocity.x , 0.f });
		}

		// Check y
		currentPos = getPosition();
		float nextY = currentPos.y + velocity.y;

		globalLeft = currentPos.x + hitbox.position.x;
		globalRight = globalLeft + hitbox.size.x;
		globalTop = nextY + hitbox.position.y;
		globalBottom = globalTop + hitbox.size.y;

		bool collisionY = false;

		if (velocity.y > 0) {
			if (map.isBlockingAtPixel(globalLeft, globalBottom) ||
				map.isBlockingAtPixel(globalRight, globalBottom)) collisionY = true;
		}
		else if (velocity.y < 0) {
			if (map.isBlockingAtPixel(globalLeft, globalTop) ||
				map.isBlockingAtPixel(globalRight, globalTop)) collisionY = true;
		}

		if (!collisionY) {
			move({ 0.f, velocity.y });
		}
	}

	void CollidingEntity::drawHitbox(sf::RenderWindow& window) {
		sf::RectangleShape rect;
		rect.setSize(hitbox.size);

		sf::Vector2f globalPos = getPosition() + hitbox.position;
		rect.setPosition(globalPos);

		rect.setFillColor(sf::Color::Transparent);
		rect.setOutlineColor(sf::Color::White);
		rect.setOutlineThickness(1.f);

		window.draw(rect);
	}

}