#include "Monster.h"

namespace RPG {

	Monster::Monster(std::string name, sf::Vector2f startPos, int16_t hp, int16_t phDef, int16_t mgDef, bool alive)
		: name(name), HP(hp), maxHP(hp), pos(startPos), phDef(phDef), mgDef(mgDef), alive(true)
	{

	}

	void Monster::takeDamage(int16_t amount) {
		HP -= amount;

		if (HP <= 0) {
			HP = 0;
			alive = false;
		}
	}

	bool Monster::isAlive() const { return alive; }

	sf::Vector2f Monster::getPosition() const { return pos; }

}