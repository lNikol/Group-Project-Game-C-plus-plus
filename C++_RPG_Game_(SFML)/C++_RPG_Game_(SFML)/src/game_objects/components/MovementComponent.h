#pragma once
#include "Component.h"

namespace RPG {
	class MovementComponent : public Component {
		float speed;
		sf::Vector2f direction;

	public:
		MovementComponent(float speed);

		void update(float dt) override;

		void setDirection(sf::Vector2f direction);
		void setSpeed(float speed);
		sf::Vector2f getVelocity() const;
	};
}

