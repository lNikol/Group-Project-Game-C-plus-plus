#pragma once
#include <SFML/Graphics.hpp>
#include "Animation.h"

namespace RPG {

	class Entity {
		sf::Vector2f pos;
		Animation* currentAnimation;
	public:
		void update(float dt);
		void draw(const sf::RenderWindow& window);
	};

}

