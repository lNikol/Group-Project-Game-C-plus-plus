#pragma once
#include "Component.h"
#include "MovementComponent.h"
#include <cassert>
#include <SFML/Graphics.hpp>

namespace RPG {
	/**
		This class requires the GameObject to have a 
		@MovementComponent assigned in order to function.
	*/
	class PlayerInputComponent : public Component {
		MovementComponent* movementComponent;
	public:
		void init() override;
		void update(float dt) override;
	};
}
