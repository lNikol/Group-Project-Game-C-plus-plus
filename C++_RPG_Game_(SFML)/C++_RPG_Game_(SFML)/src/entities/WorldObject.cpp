#include "WorldObject.h"
#include "../worldmap/AssetManager.h"

namespace RPG {

	static sf::Texture dummyTexture;

	WorldObject::WorldObject(
		const StructureDefinition* definition, 
		sf::Vector2f position,
		const AssetManager& am
	)
		: definition(definition), sprite(dummyTexture)
	{
		const Spritesheet* spritesheet = am.getSpritesheet(definition->name);
		if (spritesheet) {
			sf::IntRect rect(
				{
					static_cast<int>(definition->textureStartPos.x),
					static_cast<int>(definition->textureStartPos.y)
				},
				{
					static_cast<int>(definition->size.x),
					static_cast<int>(definition->size.y)
				}
			);
			sprite.setTexture(*spritesheet);
			sprite.setTextureRect(rect);
		}
		sprite.setPosition(position);
	}


	const StructureDefinition& WorldObject::getDefinition() const {
		return *definition;
	}

	float WorldObject::getSortY() const {
		return sprite.getPosition().y + definition->hitboxSize.y + definition->hitboxOffset.y;
	}

	sf::Vector2f WorldObject::getPosition() const {
		return sprite.getPosition();
	}

	sf::FloatRect WorldObject::getHitbox() const {
		return sf::FloatRect(
			{
				sprite.getPosition().x + definition->hitboxOffset.x, 
				sprite.getPosition().y + definition->hitboxOffset.y
			},
			{
				definition->hitboxSize.x, 
				definition->hitboxSize.y
			}
		);
	}

	void WorldObject::draw(sf::RenderWindow& window) const {
		window.draw(sprite);
		if (debugHitbox) {
			sf::Vector2f pos = getPosition() + definition->hitboxOffset;
			sf::Vector2f size = definition->hitboxSize;
			sf::RectangleShape rect(size);
			rect.setPosition(pos);
			rect.setFillColor(sf::Color::Transparent);
			rect.setOutlineColor(sf::Color::Red);
			rect.setOutlineThickness(1.f);
			window.draw(rect);
		}
	}

	void WorldObject::toggleDebugHitbox() {
		debugHitbox = !debugHitbox;
	}
}