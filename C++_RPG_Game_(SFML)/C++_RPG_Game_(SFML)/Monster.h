#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include <cstdint>

namespace RPG {
	
	class Monster {
	protected:
		sf::Vector2f pos;
		std::string name;
		int16_t HP, maxHP; // health points
		int16_t phDef, mgDef; // phDef = physical defence, mgDef = magical defence
		bool alive;

	public:
		Monster(std::string name, sf::Vector2f startPos, int16_t hp, int16_t phDef, int16_t mgDef, bool alive);
		virtual ~Monster() = default; // Generate me default version of this method
		
		virtual void update() = 0; // Pure virtual

		virtual void attack() = 0;
		void takeDamage(int16_t amount);

		virtual void draw(sf::RenderWindow& window) = 0;

		bool isAlive() const;
		sf::Vector2f getPosition() const;

	};

}