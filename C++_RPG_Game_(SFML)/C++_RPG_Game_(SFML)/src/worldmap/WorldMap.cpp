#include "WorldMap.h"
#include "core/Constants.h"
#include <cstdlib> // for rand()


namespace RPG {

	uint16_t WorldMap::getWidth() const { return width; }
	uint16_t WorldMap::getHeight() const { return height; }


	WorldMap::WorldMap(uint16_t w, uint16_t h, AssetManager& am) : width(w), height(h), assetManager(am) {
		// Memory reservation for all Tiles
		tiles.resize(width * height);

		// Default values for tiles
		for (uint16_t y = 0; y < height; ++y) {
			for (uint16_t x = 0; x < width; ++x) {
				uint32_t index = static_cast<uint32_t>(y) * width + x;
				tiles[index].pos = { static_cast<int16_t>(x), static_cast<int16_t>(y), 0 };
				
				// Always set ground as Grass
				tiles[index].groundType = StructureType::Grass;

				// Set objects (walls at borders)
				if ((x == 0 || x == width - 1) || (y == 0 || y == height - 1)) {
					tiles[index].objectType = StructureType::Wall;
				}
				else tiles[index].objectType = StructureType::None;
			}
		}
	}

	Tile& WorldMap::at(int16_t x, int16_t y) {
		// Standard formula for 1D array indexing: (y * width) + x
		// .at() is better than [] because it checks if you are out of map bounds

		if (x < 0 || x >= width || y < 0 || y >= height) {
			return tiles[0];
		}

		return tiles.at(y * width + x);
	}

	const Tile& WorldMap::at(int16_t x, int16_t y) const {
		if (x < 0 || x >= width || y < 0 || y >= height) return tiles[0];
		return tiles.at(y * width + x);
	}

	bool WorldMap::isBlockingAtPixel(float pixelX, float pixelY) const {
		int16_t tx = static_cast<int16_t>(pixelX / GameConfig::TILE_SIZE);
		int16_t ty = static_cast<int16_t>(pixelY / GameConfig::TILE_SIZE);

		// Check the borders of the map
		if (tx < 0 || tx >= width || ty < 0 || ty >= height) return true;

		const auto& def = assetManager.getDefinition(at(tx, ty).objectType);
		if (!def.blocksMovement) return false;

		if (def.collisionHeight >= GameConfig::TILE_SIZE) return true;

		float tileTopY = static_cast<float>(ty * GameConfig::TILE_SIZE);
		float solidStartY = tileTopY + (GameConfig::TILE_SIZE - def.collisionHeight);

		if (pixelY < solidStartY) {
			return false;
		}

		return true;
	}


	float WorldMap::getTileSpeedModifier(int16_t x, int16_t y) const {
		if (x < 0 || x >= width || y < 0 || y >= height) return 1.0f;

		return assetManager.getDefinition(at(x, y).groundType).speedModifier;
	}

	bool WorldMap::isInsideSpawnZone(int16_t x, int16_t y, uint8_t margin) const {
		return (x >= margin && x < (width - margin) &&
			y >= margin && y < (height - margin));
	}

	void WorldMap::refreshMonsters(const DifficultyLevel& level, uint8_t mapEdgeOffset) {
		for (int16_t y = 0; y < height; ++y) {
			for (int16_t x = 0; x < width; ++x) {
				Tile& tile = at(x, y);

				const auto& groundDef = assetManager.getDefinition(tile.groundType);

				if (groundDef.canSpawnMob && 
					isInsideSpawnZone(x, y, mapEdgeOffset) &&
					tile.objectType == StructureType::None
				) {
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

	bool WorldMap::placeStructure(int16_t startX, int16_t startY, StructureType type) {
		const auto& def = assetManager.getDefinition(type);

		// 1. Check if the structure fits within map boundaries
		if (startX < 0 || startX + def.size.width > width ||
			startY < 0 || startY + def.size.height > height) {
			return false;
		}

		// 2. Check if the area is clear (no blocking objects)
		for (int16_t y = startY; y < startY + def.size.height; ++y) {
			for (int16_t x = startX; x < startX + def.size.width; ++x) {
				if (at(x, y).objectType != StructureType::None) {
					return false;
				}
			}
		}

		// 3. Place the structure
		for (int16_t y = startY; y < startY + def.size.height; ++y) {
			for (int16_t x = startX; x < startX + def.size.width; ++x) {
				if (x == startX && y == startY) {
					// This is the anchor tile - it holds the actual structure and texture
					at(x, y).objectType = type;
				}
				else {
					// These are "under" the building - we just block movement
					at(x, y).objectType = StructureType::InvisibleBlock;
				}
			}
		}

		return true;
	}

	void WorldMap::generateObstacles(float density, const sf::Vector2f& playerStartPos, uint8_t playerSafeRadius) {
		int16_t spawnX = static_cast<int16_t>(playerStartPos.x / GameConfig::TILE_SIZE); // Assuming player starts near 5,5
		int16_t spawnY = static_cast<int16_t>(playerStartPos.y / GameConfig::TILE_SIZE);

		for (int16_t y = 0; y < height; ++y) {
			for (int16_t x = 0; x < width; ++x) {
				// Skip borders (they are already Walls)
				if (x == 0 || x == width - 1 || y == 0 || y == height - 1) continue;

				// Skip safe zone around player spawn
				if (std::abs(x - spawnX) < playerSafeRadius && std::abs(y - spawnY) < playerSafeRadius) continue;

				// Randomly place an obstacle
				if ((std::rand() % 100) < (density * 100)) {
					// Randomly choose between Tree and Rock
					StructureType randomType = (std::rand() % 2 == 0) ? StructureType::Tree : StructureType::Rock;

					// Only place if object layer is empty
					if (at(x, y).objectType == StructureType::None) {
						at(x, y).objectType = randomType;
					}
				}
			}
		}
	}

	sf::Vector2f WorldMap::findNearestSafeTile(sf::Vector2f startPos) {
		int16_t startX = static_cast<int16_t>(startPos.x / GameConfig::TILE_SIZE);
		int16_t startY = static_cast<int16_t>(startPos.y / GameConfig::TILE_SIZE);

		for (int radius = 0; radius <= 5; ++radius) {
			for (int dy = -radius; dy <= radius; ++dy) {
				for (int dx = -radius; dx <= radius; ++dx) {
					if (std::abs(dx) != radius && std::abs(dy) != radius) continue;

					int16_t checkX = startX + dx;
					int16_t checkY = startY + dy;

					if (checkX >= 0 && checkX < width && checkY >= 0 && checkY < height) {

						const Tile& t = at(checkX, checkY);

						if (t.objectType == StructureType::None && 
							t.groundType == StructureType::Grass) {
							return sf::Vector2f(
								checkX * GameConfig::TILE_SIZE,
								checkY * GameConfig::TILE_SIZE
							);
						}
					}
				}
			}
		}
		return startPos; 
	}

	void WorldMap::addNPC(std::unique_ptr<NPC> npc) {
		int16_t tx = static_cast<int16_t>(npc->getPosition().x / GameConfig::TILE_SIZE);
		int16_t ty = static_cast<int16_t>(npc->getPosition().y / GameConfig::TILE_SIZE);

		if (tx < 0 || 
			tx >= width || 
			ty < 0 || 
			ty >= height || 
			at(tx, ty).objectType != StructureType::None) 
		{

			sf::Vector2f safePos = findNearestSafeTile(npc->getPosition());

			std::cout << "[WorldMap] NPC " << npc->getName()
				<< " moved from blocked tile to safe pos: "
				<< safePos.x << "," << safePos.y << std::endl;

			npc->setPosition(safePos); 
		}

		npcs.push_back(std::move(npc));
	}

	const std::vector<std::unique_ptr<NPC>>& WorldMap::getNPCs() const { return npcs; }
	std::vector<std::unique_ptr<NPC>>& WorldMap::getNPCs() { return npcs; }
}