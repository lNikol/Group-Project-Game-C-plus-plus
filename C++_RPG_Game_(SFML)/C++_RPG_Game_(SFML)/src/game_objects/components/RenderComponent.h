#pragma once
#include "Component.h"
#include <SFML/Graphics.hpp>

namespace RPG {
	class RenderComponent : public Component {
		sf::Sprite sprite;
	public:
		RenderComponent(const Spritesheet& spritesheet);
		void draw(sf::RenderWindow& window) override;

		sf::Sprite& getSprite();
	};
}

