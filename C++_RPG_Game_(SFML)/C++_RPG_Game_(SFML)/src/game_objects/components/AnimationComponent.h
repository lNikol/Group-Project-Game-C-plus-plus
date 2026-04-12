#pragma once
#include "Component.h"
#include "RenderComponent.h"
#include "animation/Animation.h"
#include <map>
#include <string>
#include <cassert>


namespace RPG {

	/**
		This class requires the GameObject to have a
		@RenderComponent assigned in order to function.
	*/
	class AnimationComponent : public Component {
		std::map<std::string, Animation> animations;
		Animation* currentAnimation = nullptr;
		RenderComponent* renderComponent = nullptr;
	public:
		AnimationComponent() = default;
		void init() override;
		void update(float dt) override;

		void setAnimations(std::map<std::string, Animation> animations);
		void loadAnimation(const std::string& name, Animation animation);
		void play(const std::string& name);
	};
}
