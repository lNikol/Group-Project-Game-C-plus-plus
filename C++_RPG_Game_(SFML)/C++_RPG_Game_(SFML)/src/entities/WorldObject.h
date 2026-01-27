#pragma once
#include <SFML/Graphics.hpp>
#include "worldmap/StructureDefinition.h"

namespace RPG {

	class AssetManager;

	class WorldObject {
		const StructureDefinition* definition;
		sf::Sprite sprite;
	public:
		WorldObject(
			const StructureDefinition* definition,
			sf::Vector2f position,
			const AssetManager& am
		);

		virtual ~WorldObject() = default;

		const StructureDefinition& getDefinition() const;
		float getSortY() const;
		sf::FloatRect getHitbox() const;
		sf::Vector2f getPosition() const;
		void draw(sf::RenderWindow& window) const;
	};

}