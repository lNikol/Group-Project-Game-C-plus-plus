#include "AnimationControllerComponent.h"
#include "game_objects/GameObject.h"

namespace RPG {

	void AnimationControllerComponent::init() {
		movementComponent = owner->getComponent<MovementComponent>();
		animationComponent = owner->getComponent<AnimationComponent>();

		assert(movementComponent != nullptr
			&& "AnimationControllerComponent added to a GameObject missing a MovementComponent!");
		assert(animationComponent != nullptr
			&& "AnimationControllerComponent added to a GameObject missing a AnimationComponent!");
	}

	void AnimationControllerComponent::update(float dt) {
		if (!movementComponent || !animationComponent) {
			std::cerr << "MovementComponent or AnimationComponent missing in AnimationContollerComponent!\n";
			return;
		}

		sf::Vector2f vel = movementComponent->getVelocity();
		bool isMoving = (vel.x != 0.f || vel.y != 0.f);

		if (isMoving) {
			if (std::abs(vel.x) > std::abs(vel.y)) {
				facing = (vel.x > 0) ? Facing::RIGHT : Facing::LEFT;
			}
			else {
				facing = (vel.y > 0) ? Facing::DOWN : Facing::UP;
			}
		}

		if (isMoving) {
			switch (facing) {
				case Facing::UP:    animationComponent->play("run_top"); break;
				case Facing::DOWN:  animationComponent->play("run_bottom"); break;
				case Facing::LEFT:  animationComponent->play("run_left"); break;
				case Facing::RIGHT: animationComponent->play("run_right"); break;
			}
		}
		else {
			switch (facing) {
				case Facing::UP:    animationComponent->play("idle_top"); break;
				case Facing::DOWN:  animationComponent->play("idle_bottom"); break;
				case Facing::LEFT:  animationComponent->play("idle_left"); break;
				case Facing::RIGHT: animationComponent->play("idle_right"); break;
			}
		}
	}



}
