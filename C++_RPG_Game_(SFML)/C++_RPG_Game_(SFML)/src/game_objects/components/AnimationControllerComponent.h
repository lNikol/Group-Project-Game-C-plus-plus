#pragma once
#include <cassert>
#include "Component.h"
#include "MovementComponent.h"
#include "AnimationComponent.h"
#include "worldmap/enums.h"

namespace RPG {
	class AnimationControllerComponent : public Component {
		MovementComponent* movementComponent = nullptr;
		AnimationComponent* animationComponent = nullptr;
		Facing facing = Facing::DOWN;
	public:
		void init() override;
		void update(float dt) override;
	};
}
