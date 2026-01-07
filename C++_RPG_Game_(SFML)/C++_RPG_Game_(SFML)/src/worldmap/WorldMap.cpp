#include "WorldMap.h"
#include "core/GameConfig.h"
#include <cstdlib> // for rand()

uint16_t RPG::WorldMap::getWidth() const { return width; }
uint16_t RPG::WorldMap::getHeight() const { return height; }


RPG::WorldMap::WorldMap(uint16_t w, uint16_t h, AssetManager& am) : width(w), height(h), assetManager(am) {
	// Memory reservation for all Tiles
	tiles.resize(width * height);
	
	// Default values for tiles
	for (uint16_t y = 0; y < height; ++y) {
		for (uint16_t x = 0; x < width; ++x) {
			uint32_t index = static_cast<uint32_t>(y) * width + x;
			tiles[index].pos = { static_cast<int16_t>(x), static_cast<int16_t>(y), 0 };

			// bounds
			if ((x == 0 || x == width - 1) || (y == 0 || y == height - 1)) {
				tiles[index].structure = StructureType::Wall;
			}
			// internal
			else tiles[index].structure = StructureType::Grass;
		}
	}
}

RPG::Tile& RPG::WorldMap::at(int16_t x, int16_t y){
	// Standard formula for 1D array indexing: (y * width) + x
	// .at() is better than [] because it checks if you are out of map bounds
	
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return tiles[0];
	}

	return tiles.at(y * width + x);
}

const RPG::Tile& RPG::WorldMap::at(int16_t x, int16_t y) const {
	if (x < 0 || x >= width || y < 0 || y >= height) return tiles[0]; 
	return tiles.at(y * width + x);
}

bool RPG::WorldMap::isBlockingAtPixel(float pixelX, float pixelY) const {
	int16_t tx = static_cast<int16_t>(pixelX / GameConfig::TILE_SIZE);
	int16_t ty = static_cast<int16_t>(pixelY / GameConfig::TILE_SIZE);

	// Check the borders of the map
	if (tx < 0 || tx >= width || ty < 0 || ty >= height) return true;

	return assetManager.getDefinition(at(tx, ty).structure).blocksMovement;
}


float RPG::WorldMap::getTileSpeedModifier(int16_t x, int16_t y) const {
	if (x < 0 || x >= width || y < 0 || y >= height) return 1.0f;

	return assetManager.getDefinition(at(x, y).structure).speedModifier;
}

bool RPG::WorldMap::isInsideSpawnZone(int16_t x, int16_t y, int16_t margin) const{
	return (x >= margin && x < (width - margin) &&
		y >= margin && y < (height - margin));
}

void RPG::WorldMap::refreshMonsters(const DifficultyLevel& level) {
	int16_t margin = 2;

	for (int16_t y = 0; y < height;  ++y) {
		for (int16_t x = 0; x < width; ++x) {
			Tile& tile = at(x, y);
			const auto& def = assetManager.getDefinition(tile.structure);

			if (def.canSpawnMob && isInsideSpawnZone(x, y, margin) && tile.isAvailableForSpawn()) {
				if (tile.isAvailableForSpawn()) {
					if ((std::rand() % 100) < level.spawnChance) {
						tile.hasMonster = true;
						tile.localDangerLvl = level.dangerLvl;
					}
				}
			}
		}
	}
}

bool RPG::WorldMap::placeStructure(int16_t startX, int16_t startY, StructureType type) {
	const auto& def = assetManager.getDefinition(type);

	// 1. Check if the structure fits within map boundaries
	if (startX < 0 || startX + def.size.width > width ||
		startY < 0 || startY + def.size.height > height) {
		return false;
	}

	// 2. Optional: Check if the area is clear (only Grass/None allowed)
	for (int16_t y = startY; y < startY + def.size.height; ++y) {
		for (int16_t x = startX; x < startX + def.size.width; ++x) {
			StructureType current = at(x, y).structure;
			// TODO remake this if coznot only on grass it can be spawned
			if (current != StructureType::Grass && current != StructureType::None) {
				return false; // Something is already here
			}
		}
	}

	// 3. Place the structure
	for (int16_t y = startY; y < startY + def.size.height; ++y) {
		for (int16_t x = startX; x < startX + def.size.width; ++x) {
			if (x == startX && y == startY) {
				// This is the anchor tile - it holds the actual structure and texture
				at(x, y).structure = type;
			}
			else {
				// These are "under" the building - we just block movement
				at(x, y).structure = StructureType::InvisibleBlock;
			}
		}
	}

	return true;
}



void RPG::WorldMap::generateObstacles(float density) {
	int16_t safeMargin = 2;
	int16_t spawnX = 5; // Assuming player starts near 5,5
	int16_t spawnY = 5;

	for (int16_t y = 0; y < height; ++y) {
		for (int16_t x = 0; x < width; ++x) {
			// Skip borders (they are already Walls)
			if (x == 0 || x == width - 1 || y == 0 || y == height - 1) continue;

			// Skip safe zone around player spawn
			if (std::abs(x - spawnX) < safeMargin && std::abs(y - spawnY) < safeMargin) continue;

			// Randomly place an obstacle
			if ((std::rand() % 100) < (density * 100)) {
				// Randomly choose between Tree and Rock
				StructureType randomType = (std::rand() % 2 == 0) ? StructureType::Tree : StructureType::Rock;

				// Only place if the tile is currently Grass
				if (at(x, y).structure == StructureType::Grass) {
					at(x, y).structure = randomType;
				}
			}
		}
	}
}