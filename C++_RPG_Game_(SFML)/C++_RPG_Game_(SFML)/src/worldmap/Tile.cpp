#include "Tile.h"

namespace RPG {

	Tile::Tile() : 
		groundType(StructureType::Grass), 
		objectType(StructureType::None),
		pos({ 0, 0, 0 }) 
	{
	}

	Tile::Tile(StructureType ground, StructureType object, int16_t x, int16_t y, int16_t z) : 
		groundType(ground), 
		objectType(object), 
		pos({ x,y,z }) 
	{
	}


	bool Tile::isAvailableForSpawn() const {
		return !hasPlayer && !hasMonster && objectType == StructureType::None;
	}

	void Tile::clearMonster() {
		hasMonster = false;

	}
}