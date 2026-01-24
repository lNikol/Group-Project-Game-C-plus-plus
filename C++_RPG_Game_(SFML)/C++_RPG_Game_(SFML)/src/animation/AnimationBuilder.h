#pragma once
#include "Animation.h"
#include <cassert>

namespace RPG {

	class AnimationBuilder {
		Animation animation;

	public:
		AnimationBuilder& spritesheet(const Spritesheet* spritesheet);
		AnimationBuilder& frameStartPos(const sf::Vector2u& vec);
		AnimationBuilder& frameSize(const sf::Vector2u& vec);
		AnimationBuilder& frameCount(uint8_t count);
		AnimationBuilder& frameGap(uint8_t px);
		AnimationBuilder& looped(bool state);
		AnimationBuilder& frameDuration(float duration);
		Animation build();
	};

}

