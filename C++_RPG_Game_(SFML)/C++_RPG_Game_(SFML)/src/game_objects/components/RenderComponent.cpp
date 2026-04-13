#include "RenderComponent.h"
#include "game_objects/GameObject.h"

namespace RPG {

	RenderComponent::RenderComponent(const Spritesheet& spritesheet) 
		: sprite(spritesheet)
	{
	}

	RenderComponent::RenderComponent(const Spritesheet& spritesheet, sf::Vector2u textureStartPos, sf::Vector2u textureSize)
		: sprite(spritesheet)
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
	}

	sf::Sprite& RenderComponent::getSprite() {
		return sprite;
	}

}
