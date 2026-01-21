#include "AnimationBuilder.h"

namespace RPG {

	AnimationBuilder& AnimationBuilder::spritesheet(const Spritesheet* spritesheet) {
		animation.spritesheet = spritesheet;
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

	AnimationBuilder& AnimationBuilder::frameGapPx(uint8_t px) {
		animation.frameGapPx = px;
		return *this;
	}

	AnimationBuilder& AnimationBuilder::looped(bool state) {
		animation.looped = state;
		return *this;
	}

	Animation AnimationBuilder::build() {
		animation.currentFrame.position = static_cast<sf::Vector2i>(animation.frameStartPos);
		animation.currentFrame.size = static_cast<sf::Vector2i>(animation.frameSize);

		return std::move(animation);
	}
}