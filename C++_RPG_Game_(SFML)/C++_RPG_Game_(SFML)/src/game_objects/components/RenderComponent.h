#pragma once
#include "Component.h"
#include <SFML/Graphics.hpp>

namespace RPG {
	class RenderComponent : public Component {
		sf::Sprite sprite;
		bool showDebug = false;
	public:
		RenderComponent(const Spritesheet& spritesheet);
		RenderComponent(const Spritesheet& spritesheet, sf::Vector2u textureStartPos, sf::Vector2u textureSize);
		void draw(sf::RenderWindow& window) override;

		sf::Sprite& getSprite();

		void setDebug(bool state);
		void toggleDebug();
	};
}

