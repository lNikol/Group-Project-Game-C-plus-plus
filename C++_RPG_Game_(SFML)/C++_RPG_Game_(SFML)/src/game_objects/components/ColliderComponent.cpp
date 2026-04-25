#include "ColliderComponent.h"
#include "game_objects/GameObject.h"

namespace RPG {

	ColliderComponent::ColliderComponent(float width, float height, float offsetX, float offsetY) {
		localHitbox.position.x = offsetX;
		localHitbox.position.y = offsetY;
		localHitbox.size.x = width;
		localHitbox.size.y = height;
	}

	ColliderComponent::ColliderComponent(sf::Vector2f size, sf::Vector2f offset)
		: ColliderComponent(size.x, size.y, offset.x, offset.y)
	{
	}

	void ColliderComponent::draw(sf::RenderWindow& window) {
		if (!showDebug || !owner) return;

		sf::RectangleShape rect({ localHitbox.size.x, localHitbox.size.y });
		sf::Vector2f globalPos = owner->getPosition() + sf::Vector2f(
			localHitbox.position.x, 
			localHitbox.position.y
		);

		rect.setPosition(globalPos);
		rect.setFillColor(sf::Color::Transparent);
		rect.setOutlineColor(sf::Color::Red);
		rect.setOutlineThickness(1.f);

		window.draw(rect);
	}

	sf::FloatRect ColliderComponent::getGlobalHitbox() const {
		if (!owner) {
			std::cerr << "No owner has been assigned to this component; Returning local hitbox instead";
			return localHitbox;
		}

		sf::Vector2f pos = owner->getPosition();
		return sf::FloatRect(
			{ pos.x + localHitbox.position.x, pos.y + localHitbox.position.y, },
			{ localHitbox.size.x, localHitbox.size.y }
		);
	}

	sf::FloatRect ColliderComponent::getLocalHitbox() const {
		return localHitbox;
	}

	void ColliderComponent::setDebug(bool show) {
		showDebug = show;
	}

	void ColliderComponent::toggleDebug() {
		showDebug = !showDebug;
	}

}
