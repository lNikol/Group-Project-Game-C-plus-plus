#include "Tile.h"

RPG::Tile::Tile()
	: structure(StructureType::Grass), pos({ 0, 0, 0 }) {}

RPG::Tile::Tile(StructureType type, int16_t x, int16_t y, int16_t z)
	: structure(type), pos({ x,y,z }) {}


bool RPG::Tile::isAvailableForSpawn() const {
	return !hasPlayer && !hasMonster;
}

void RPG::Tile::clearMonster() {
	hasMonster = false;

}