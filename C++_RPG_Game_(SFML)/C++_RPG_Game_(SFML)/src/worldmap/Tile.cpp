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
		return !blocksMovement;
	}

	bool Tile::isAvailableForPlace() const {
		return !blocksPlacement;
	}

	bool Tile::canAccept(PlacementLayer newLayer) const {
		if (blocksPlacement) return false; // walls block whole tile
		// objects (e.g chests) block only movement, decorations (flowers) can be placed when we have object
		if (newLayer == PlacementLayer::Object && blocksMovement) return false; 
		return true;
	}


	void Tile::release() {
		blocksMovement = false;
		blocksPlacement = false;
		residentObjects.clear();
	}
	/*
	void Tile::clearMonster() {
		hasMonster = false;
		release();
	}
	*/
}