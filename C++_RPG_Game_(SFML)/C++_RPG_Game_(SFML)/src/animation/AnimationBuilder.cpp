#include "AnimationBuilder.h"

namespace RPG {

	AnimationBuilder& AnimationBuilder::spritesheet(const Spritesheet* spritesheet) {
		animation.spritesheet = spritesheet;
		return *this;
	}

	AnimationBuilder& AnimationBuilder::base(Animation animation) {
		this->animation = std::move(animation);
		return *this;
	}

	AnimationBuilder& AnimationBuilder::frameStartPos(const sf::Vector2u& vec) {
		animation.frameStartPos = vec;
		return *this;
	}

	AnimationBuilder& AnimationBuilder::frameSize(const sf::Vector2u& vec) {
		animation.frameSize = vec;
		return *this;
	}

	AnimationBuilder& AnimationBuilder::frameCount(uint8_t count) {
		animation.frameCount = count;
		return *this;
	}

	AnimationBuilder& AnimationBuilder::frameGap(uint8_t px) {
		animation.frameGap = px;
		return *this;
	}

	AnimationBuilder& AnimationBuilder::looped(bool state) {
		animation.looped = state;
		return *this;
	}

	AnimationBuilder& AnimationBuilder::frameDuration(float duration) {
		animation.frameDuration = duration;
		return *this;
	}

	Animation AnimationBuilder::build() {
		animation.currentFrame.position = static_cast<sf::Vector2i>(animation.frameStartPos);
		animation.currentFrame.size = static_cast<sf::Vector2i>(animation.frameSize);

		std::cout << "[Build] Size: " << animation.frameSize.x << "x" << animation.frameSize.y
			<< " | Count: " << (int)animation.frameCount << "\n";

		assert(animation.spritesheet != nullptr && "Cannot build animation with no spritesheet!");
		assert(animation.frameCount > 0 && "Animation must have at least 1 frame!");
		assert((animation.frameSize.x > 0 && animation.frameSize.y > 0) && "Animation frame size must be positive!");

		Animation result = std::move(animation);
		animation = Animation();
		return std::move(result);
	}
}