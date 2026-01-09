#pragma once
#include "core/Constants.h"
#include "core/SpritesheetManager.h"
#include "WorldMap.h"
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <cmath>

namespace RPG {

	class Player {
	private:
		sf::Vector2f position; // Position in pixels
		float moveSpeed = 250.0f; // px/s

		GameConfig::Faction faction = GameConfig::Faction::NEUTRAL;

		sf::Vector2f getInputDirection(const sf::RenderWindow& window) const;
	public:
		Player(float startX, float startY);

		/**
		 * @brief Updates player logic and movement
		 * @param deltaTime Time elapsed since last frame
		 * @param worldMap Reference to check collisions and speed modifiers
		 */
		void update(float deltaTime, const WorldMap& worldMap, const sf::RenderWindow& window);
		void draw(sf::RenderWindow& window, const SpritesheetManager& sm);
		sf::Vector2f getPosition() const;
	};
}