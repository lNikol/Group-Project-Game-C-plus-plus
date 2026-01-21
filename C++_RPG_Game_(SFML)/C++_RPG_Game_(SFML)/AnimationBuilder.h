#pragma once
#include "Animation.h"

namespace RPG {

	class AnimationBuilder {
		Animation animation;

	public:
		AnimationBuilder& spritesheet(const Spritesheet* spritesheet);
		AnimationBuilder& frameStartPos(const sf::Vector2u& vec);
		AnimationBuilder& frameSize(const sf::Vector2u& vec);
		AnimationBuilder& frameCount(uint8_t count);
		AnimationBuilder& frameGapPx(uint8_t px);
		AnimationBuilder& looped(bool state);
		Animation build();
	};

}

