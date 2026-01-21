#pragma once
#include "Constants.h"
#include <cstdint>

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
		uint8_t frameGapPx;
		bool looped;
		sf::IntRect currentFrame;

		Animation();
	public:
		static AnimationBuilder builder();

		void update(float dt);
		const sf::IntRect& getCurrentFrame() const;

		friend std::ostream& operator << (std::ostream& out, const Animation& a) {
			out << "Start pos: " << a.frameStartPos.x << " " << a.frameStartPos.y << "\n" \
				<< "Size: " << a.frameSize.x << " " << a.frameSize.y << "\n" \
				<< "Frame count: " << (unsigned int)a.frameCount << "\n" \
				<< "Frame gap: " << (unsigned int)a.frameGapPx << "\n" \
				<< "Looped: " << a.looped << "\n";
			return out;
		}
	};

}

