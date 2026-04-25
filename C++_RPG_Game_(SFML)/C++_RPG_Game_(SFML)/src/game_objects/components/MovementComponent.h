#pragma once
#include "Component.h"

namespace RPG {

	class WorldMap;

	class MovementComponent : public Component {
		float speed;
		sf::Vector2f direction;
		const WorldMap& worldmap;

	public:
		MovementComponent(const WorldMap& worldmap, float speed);

		void update(float dt) override;

		void setDirection(sf::Vector2f direction);
		void setSpeed(float speed);
		sf::Vector2f getVelocity() const;
	};
}

