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
		Player(sf::Vector2f pos);

		/**
		 * @brief Updates player logic and movement
		 * @param deltaTime Time elapsed since last frame
		 * @param worldMap Reference to check collisions and speed modifiers
		 */
		void update(float deltaTime, const WorldMap& worldMap, const sf::RenderWindow& window);
		void draw(sf::RenderWindow& window, const SpritesheetManager& sm);
		sf::Vector2f getPosition() const;

		/**
		 * @brief Updates the player's world position and synchronizes the sprite.
		 * * Use this method for teleporting the player between maps or responding
		 * to scene changes.
		 * * @param pos The new 2D coordinates (in pixels) for the player.
		 */
		void setPosition(sf::Vector2f pos);
	};
}