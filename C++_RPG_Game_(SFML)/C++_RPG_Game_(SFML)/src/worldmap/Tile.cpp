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

	bool Tile::isAvailableForPlace() const {
		return !isOccupied;
	}

	void Tile::occupy() {
		isOccupied = true;
	}

	void Tile::release() {
		isOccupied = false;
		residentObjects.clear();
	}
	/*
	void Tile::clearMonster() {
		hasMonster = false;
		release();
	}
	*/
}