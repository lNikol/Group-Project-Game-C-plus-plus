#include "WorldMap.h"
#include "core/Constants.h"
#include <cstdlib> // for rand()


namespace RPG {

	uint32_t WorldMap::getWidth() const { return width; }
	uint32_t WorldMap::getHeight() const { return height; }

	WorldMap::WorldMap(uint32_t w, uint32_t h) : width(w), height(h),
		width_gen(w - GameConfig::WORLD_GENERATE_MARGIN), height_gen(h - GameConfig::WORLD_GENERATE_MARGIN) {

		// Memory reservation for all Tiles
		tiles.resize(width * height);

		// Default values for tiles
		for (uint32_t y = 0; y < height; ++y) {
			for (uint32_t x = 0; x < width; ++x) {
				uint32_t index = static_cast<uint32_t>(y) * width + x;
				tiles[index].pos = { static_cast<int32_t>(x), static_cast<int32_t>(y), 0 };
				
				// Always set ground as Grass
				tiles[index].groundType = StructureType::Grass;

				// Set objects (walls at borders)
				if ((x == 0 || x == width - 1) || (y == 0 || y == height - 1)) {
					// TODO: Place wall structure
					tiles[index].isWalkable = false;
					tiles[index].occupy();
				}
			}
		}
	}

	Tile& WorldMap::at(int32_t x, int32_t y) {
		// Standard formula for 1D array indexing: (y * width) + x
		// .at() is better than [] because it checks if you are out of map bounds
		if (x < 0 || x >= width || y < 0 || y >= height) {
			return tiles[0];
		}
		return tiles.at(y * width + x);
	}

	const Tile& WorldMap::at(int32_t x, int32_t y) const {
		if (x < 0 || x >= width || y < 0 || y >= height) return tiles[0];
		return tiles.at(y * width + x);
	}

	bool WorldMap::isBlockingAtPixel(float pixelX, float pixelY) const {
		int32_t tx = static_cast<int32_t>(pixelX / GameConfig::TILE_SIZE);
		int32_t ty = static_cast<int32_t>(pixelY / GameConfig::TILE_SIZE);

		// 1. Check Map Boundaries
		if (tx < 0 || tx >= width || ty < 0 || ty >= height) return true;

		const Tile& tile = at(tx, ty);

		// 2. Check Terrain (Water/Walls)
		// This is still a "whole tile" block, which is correct for water.
		if (!tile.isWalkable) return true;

		// 3. Check Objects (Pixel Perfect)
		// Only check the objects registered to THIS tile.
		for (const auto* obj : tile.residentObjects) {
			if (obj->getHitbox().contains({ pixelX, pixelY })) {
				return true; // HIT!
			}
		}

		return false;
	}


	float WorldMap::getTileSpeedModifier(int32_t x, int32_t y) const {
		if (x < 0 || x >= width || y < 0 || y >= height) return 1.0f;

		return at(x, y).speedModifier;
	}

	bool WorldMap::isInsideSpawnZone(int32_t x, int32_t y, uint8_t margin) const {
		return (x >= margin && x < (width - margin) &&
			y >= margin && y < (height - margin));
	}

	void WorldMap::refreshMonsters(const DifficultyLevel& level, uint8_t mapEdgeOffset) {

		// Monsters can also appear all over the map
		// When obstacles are only generated in (width_gen&height_gen) zone

		for (uint32_t y = 0; y < height; ++y) {
			for (uint32_t x = 0; x < width; ++x) {
				Tile& tile = at(x, y);

				// TODO : check if it's written correctly

				// Note: We need to get the Definition of the GROUND to check 'canSpawnMob'
				// But your Tile currently stores groundType as Enum. 
				// You might need a helper in AssetManager to get Def from Enum, or just check type directly.
				bool canSpawn = (tile.groundType == StructureType::Grass || tile.groundType == StructureType::Sand);

				if (canSpawn &&
					isInsideSpawnZone(x, y, mapEdgeOffset) &&
					tile.isAvailableForSpawn() // Checks !isOccupied
					) {
					if ((std::rand() % 100) < level.spawnChance) {
						// Spawn logic (add to NPC list)
						// tile.hasMonster = true; // Use this flag if you kept it
					}
				}
			}
		}
	}

	bool WorldMap::placeStructureAtTile(int32_t tileX, int32_t tileY, const std::string& name) {
		return placeStructure(
			static_cast<float>(tileX * GameConfig::TILE_SIZE),
			static_cast<float>(tileY * GameConfig::TILE_SIZE),
			name
		);
	}

	bool WorldMap::placeStructure(float worldX, float worldY, const std::string& id) {
		const StructureDefinition& def = AssetManager::getInstance().getDefinition(id);

		// 1. Create Object (on Heap via unique_ptr)
		auto newObj = std::make_unique<WorldObject>(&def, sf::Vector2f(worldX, worldY));

		sf::FloatRect hitbox = newObj->getHitbox();

		int32_t startTx = static_cast<int32_t>(hitbox.position.x / GameConfig::TILE_SIZE);
		int32_t startTy = static_cast<int32_t>(hitbox.position.y / GameConfig::TILE_SIZE);
		int32_t endTx = static_cast<int32_t>((hitbox.position.x + hitbox.size.x - 0.1f) / GameConfig::TILE_SIZE);
		int32_t endTy = static_cast<int32_t>((hitbox.position.y + hitbox.size.y - 0.1f) / GameConfig::TILE_SIZE);

		if (startTx < 0 || startTy < 0 || endTx >= width_gen || endTy >= height_gen) {
			// Beyond borders
			return false;
		}

		// If any of those tiles is occupated - abort placement
		for (int32_t y = startTy; y <= endTy; ++y) {
			for (int32_t x = startTx; x <= endTx; ++x) {
				if (!at(x, y).isAvailableForPlace()) {
					return false; 
				}
			}
		}

		WorldObject* objPtr = newObj.get(); // Keep a raw pointer for the tiles
		// 2. Register to Grid (Spatial Partitioning)

		for (int32_t y = startTy; y <= endTy; ++y) {
			for (int32_t x = startTx; x <= endTx; ++x) {
				at(x, y).occupy(); 
				// Add pointer to all covered tiles
				if (def.meta.blocksMovement) {
					at(x, y).residentObjects.push_back(objPtr);
				}
			}
		}

		// 3. Store ownership
		structures.push_back(std::move(newObj));
		return true;
		
	}

	void WorldMap::generateObstacles(float density, const sf::Vector2f& playerStartPos, uint8_t playerSafeRadius) {
        int32_t spawnX = static_cast<int32_t>(playerStartPos.x / GameConfig::TILE_SIZE);
        int32_t spawnY = static_cast<int32_t>(playerStartPos.y / GameConfig::TILE_SIZE);
		for (uint32_t y = GameConfig::WORLD_GENERATE_MARGIN; y <= height_gen; ++y) {
			for (uint32_t x = GameConfig::WORLD_GENERATE_MARGIN; x <= width_gen; ++x) {

                // Safe Zone Check
                if (std::abs((int)x - spawnX) < playerSafeRadius && 
                    std::abs((int)y - spawnY) < playerSafeRadius) continue;

                if ((std::rand() % 100) < (density * 100)) {
                    // Randomly pick a structure ID
                    std::string id = (std::rand() % 2 == 0) ? "big_tree" : "rock";
                    
                    // Try to place it (will fail if blocked)
					if (!placeStructureAtTile(x, y, id)) {
						// std::cout << "[Map WxH]: " << width <<" " << height << " Failed to place " << id << " at(x, y): " << x << " " << y << std::endl;
					}
                }
            }
        }
    }

	sf::Vector2f WorldMap::findNearestSafeTile(sf::Vector2f startPos) {
		int32_t startX = static_cast<int32_t>(startPos.x / GameConfig::TILE_SIZE);
		int32_t startY = static_cast<int32_t>(startPos.y / GameConfig::TILE_SIZE);

		// Spiral search or simple radius
		for (int r = 0; r < 5; ++r) {
			for (int dy = -r; dy <= r; ++dy) {
				for (int dx = -r; dx <= r; ++dx) {
					int32_t tx = startX + dx;
					int32_t ty = startY + dy;
					if (tx >= 0 && tx <= width_gen && ty >= 0 && ty <= height_gen) {
						if (at(tx, ty).isAvailableForSpawn()) {
							return sf::Vector2f(tx * GameConfig::TILE_SIZE, ty * GameConfig::TILE_SIZE);
						}
					}
				}
			}
		}
		return startPos;
	}

	void WorldMap::addNPC(std::unique_ptr<NPC> npc) {
		int32_t tx = static_cast<int32_t>(npc->getPosition().x / GameConfig::TILE_SIZE);
		int32_t ty = static_cast<int32_t>(npc->getPosition().y / GameConfig::TILE_SIZE);

		if (tx < 0 || 
			tx >= width_gen || 
			ty < 0 || 
			ty >= height_gen) 
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

	const std::vector<std::unique_ptr<WorldObject>>& WorldMap::getStructures() const {
		return structures;
	}
	void WorldMap::toggleDebugHitbox() {
		for (auto& e : structures) {
			e->toggleDebugHitbox();
		}
	}
}