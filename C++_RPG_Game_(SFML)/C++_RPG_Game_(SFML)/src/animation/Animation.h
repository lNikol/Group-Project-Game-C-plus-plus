#pragma once
#include <cstdint>
#include <iostream>
#include "core/Constants.h"

namespace RPG {

	class Animation {
		// Forward declaration, gives AnimationBuilder 
		// class full access to private members
		friend class AnimationBuilder;
	private:
		const Spritesheet* spritesheet;
		sf::Vector2u frameStartPos;
		sf::Vector2u frameSize;
		uint8_t frameCount;
		uint8_t frameGap;
		bool looped;
		float frameDuration;

		float timer;
		uint8_t currentFrameIndex;
		sf::IntRect currentFrame;
		bool finished;
		bool reversed = false;

	public:
		Animation();
		virtual ~Animation();
		static AnimationBuilder builder();

		void update(float dt);
		void reset();

		void setReversed(bool rev);
		bool isReversed() const;
		void setFinished(bool fin);

		const sf::IntRect& getCurrentFrame() const;
		const Spritesheet* getSpritesheet() const;
		bool isFinished() const;
		const uint8_t getCurrentFrameIndex() const;

		friend std::ostream& operator << (std::ostream& out, const Animation& a) {
			out << "Start pos: " << a.frameStartPos.x << " " << a.frameStartPos.y << "\n" \
				<< "Size: " << a.frameSize.x << " " << a.frameSize.y << "\n" \
				<< "Frame count: " << (unsigned int)a.frameCount << "\n" \
				<< "Frame gap: " << (unsigned int)a.frameGap << "\n" \
				<< "Looped: " << a.looped << "\n";
			return out;
		}
	};

}

