#pragma once
#include <SFML/Graphics.hpp>
#include "AnimatedEntity.h"
#include "worldmap/WorldMap.h"

namespace RPG {

	class CollidingEntity : public AnimatedEntity {
		
		sf::FloatRect hitbox;

	public:
		CollidingEntity();
		virtual ~CollidingEntity();

		void setHitbox(float width, float height, float offsetY);
		void moveWithCollision(sf::Vector2f velocity, const WorldMap& map);
		void drawHitbox(sf::RenderWindow& window);
	};

}
