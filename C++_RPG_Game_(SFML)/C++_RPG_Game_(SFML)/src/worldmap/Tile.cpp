#include "Tile.h"

namespace RPG {

	Tile::Tile()
		: structure(StructureType::Grass), pos({ 0, 0, 0 }) {
	}

	Tile::Tile(StructureType type, int16_t x, int16_t y, int16_t z)
		: structure(type), pos({ x,y,z }) {
	}


	bool Tile::isAvailableForSpawn() const {
		return !hasPlayer && !hasMonster;
	}

	void Tile::clearMonster() {
		hasMonster = false;

	}
}