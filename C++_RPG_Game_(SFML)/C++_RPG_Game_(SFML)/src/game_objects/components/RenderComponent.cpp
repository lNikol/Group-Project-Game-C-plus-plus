#include "RenderComponent.h"
#include "game_objects/GameObject.h"

namespace RPG {

	RenderComponent::RenderComponent(const Spritesheet& spritesheet) 
		: sprite(spritesheet)
	{
	}

	RenderComponent::RenderComponent(const Spritesheet& spritesheet, sf::Vector2u textureStartPos, sf::Vector2u textureSize)
		: RenderComponent(spritesheet)
	{
		sprite.setTextureRect({
			{ (int) textureStartPos.x, (int) textureStartPos.y },
			{ (int) textureSize.x, (int) textureSize.y }
		});
	}

	void RenderComponent::draw(sf::RenderWindow& window) {
		if (owner) {
			sprite.setPosition(owner->getPosition());
		}

		window.draw(sprite);

		if (owner) {
			// Create a tiny circle (Radius = 2 pixels)
			sf::CircleShape originDot(2.f);

			// Center the dot on itself so it's perfectly accurate
			originDot.setOrigin({ 2.f, 2.f });

			// Move the dot to the exact GameObject position (which is the sprite's origin)
			originDot.setPosition(owner->getPosition());

			// Make it a highly visible, ugly color that stands out (Magenta/Hot Pink)
			originDot.setFillColor(sf::Color::Magenta);

			// Draw it
			window.draw(originDot);
		}
	}

	sf::Sprite& RenderComponent::getSprite() {
		return sprite;
	}

}
