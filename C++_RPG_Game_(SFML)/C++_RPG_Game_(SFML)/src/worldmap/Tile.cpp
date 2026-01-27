#include "Tile.h"

namespace RPG {

	Tile::Tile() : 
		groundType(StructureType::Grass), 
		pos({ 0, 0, 0 }) 
	{
	}

	Tile::Tile(StructureType ground, int32_t x, int32_t y, int32_t z) : 
		groundType(ground), 
		pos({ x,y,z }) 
	{
	}


	bool Tile::isAvailableForSpawn() const {
		return !isOccupied && isWalkable;
	}

	/*
	void Tile::clearMonster() {
		hasMonster = false;
	}
	*/
}