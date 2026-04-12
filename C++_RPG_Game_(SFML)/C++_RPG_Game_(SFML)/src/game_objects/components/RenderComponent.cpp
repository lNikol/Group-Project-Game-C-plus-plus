#include "RenderComponent.h"
#include "game_objects/GameObject.h"

namespace RPG {

	RenderComponent::RenderComponent(const Spritesheet& spritesheet) 
		: sprite(spritesheet)
	{
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
