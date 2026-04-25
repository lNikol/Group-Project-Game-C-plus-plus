#include "RenderComponent.h"
#include "game_objects/GameObject.h"

namespace RPG {

	RenderComponent::RenderComponent(const Spritesheet& spritesheet) 
		: sprite(spritesheet)
	{
		sf::FloatRect bounds = sprite.getLocalBounds();
		sprite.setOrigin({ bounds.size.x / 2.f, bounds.size.y });
	}

	RenderComponent::RenderComponent(const Spritesheet& spritesheet, sf::Vector2u textureStartPos, sf::Vector2u textureSize)
		: RenderComponent(spritesheet)
	{
		sprite.setTextureRect({
			{ (int) textureStartPos.x, (int) textureStartPos.y },
			{ (int) textureSize.x, (int) textureSize.y }
		});

		sprite.setOrigin({ textureSize.x / 2.f, (float)textureSize.y });
	}

	void RenderComponent::draw(sf::RenderWindow& window) {
		if (owner) {
			sprite.setPosition(owner->getPosition());
		}

		window.draw(sprite);

		if (owner && showDebug) {
			sf::CircleShape originDot(2.f);
			originDot.setOrigin({ 2.f, 2.f });
			originDot.setPosition(owner->getPosition());
			originDot.setFillColor(sf::Color::Magenta);
			window.draw(originDot);
		}
	}

	sf::Sprite& RenderComponent::getSprite() {
		return sprite;
	}

	void RenderComponent::setDebug(bool state) {
		showDebug = state;
	}

	void RenderComponent::toggleDebug() {
		showDebug = !showDebug;
	}

}
