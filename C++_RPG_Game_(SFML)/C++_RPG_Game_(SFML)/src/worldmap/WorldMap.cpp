#include "WorldMap.h"
#include "core/Constants.h"
#include <fstream>
#include <filesystem>

// TODO : write helper for parser to avoid code duplication between infinite and standard map loading
// and other helper class for loading/saving maps 

namespace RPG {

	uint32_t WorldMap::getWidth() const { return width; }
	uint32_t WorldMap::getHeight() const { return height; }
	sf::Vector2f WorldMap::getSpawnPoint() const { return spawnPoint; }
	bool WorldMap::getHasSpawnPoint() const { return hasSpawnPoint; }

	std::vector<std::unique_ptr<GameObject>>& WorldMap::getGameObjects() {
		return gameObjects;
	}

	const std::vector<std::unique_ptr<GameObject>>& WorldMap::getGameObjects() const {
		return gameObjects;
	}

	void WorldMap::addGameObject(std::unique_ptr<GameObject> go) {
		gameObjects.push_back(std::move(go));
	}

	WorldMap::WorldMap(uint32_t w, uint32_t h, AssetManager& am) : width(w), height(h), assetManager(am) {

	WorldMap::WorldMap(uint32_t w, uint32_t h, AssetManager& am) : width(w), height(h), assetManager(am),
		width_gen(w - GameConfig::WORLD_GENERATE_MARGIN), height_gen(h - GameConfig::WORLD_GENERATE_MARGIN) {
		initializeGrid();
		generateBorders();
	}

	void WorldMap::setPlayerInitialPosition(float x, float y) {
		this->spawnPoint = { x, y };
		hasSpawnPoint = true;
	}

	void WorldMap::initializeGrid() {
		tiles.clear();
		// Memory reservation for all Tiles
		tiles.resize(width * height);

		for (uint32_t y = 0; y < height; ++y) {
			for (uint32_t x = 0; x < width; ++x) {
				auto& tile = at(x, y);
				tile.pos = { static_cast<int32_t>(x), static_cast<int32_t>(y), 0 };

				tile.groundType = StructureType::Grass;
				tile.blocksMovement = false;
				tile.blocksPlacement = false;
				tile.residentObjects.clear(); 
			}
		}
	}

	void WorldMap::generateBorders() {
		std::string wallId = "IconSet_408"; // TODO: change on 'wall' when will be a sprite

		for (uint32_t x = 0; x < width; ++x) {
			// Top and bottom borders
			placeStructure(x * GameConfig::TILE_SIZE, 0, wallId);
			placeStructure(x * GameConfig::TILE_SIZE, (height - 1) * GameConfig::TILE_SIZE, wallId);
		}

		for (uint32_t y = 1; y < height - 1; ++y) {
			// Left and rigth borders
			placeStructure(0, y * GameConfig::TILE_SIZE, wallId);
			placeStructure((width - 1) * GameConfig::TILE_SIZE, y * GameConfig::TILE_SIZE, wallId);
		}
	}

	void WorldMap::assignGroundType(uint32_t tx, uint32_t ty, uint32_t gid) {
		if (tx >= 0 && tx < width && ty >= 0 && ty < height) {
			std::string assetName = assetManager.getNameById(gid - 1);
			if (!assetName.empty()) {
				tiles[ty * width + tx].groundType = assetManager.getDefinition(assetName).type;
			}
		}
	}
	
	void WorldMap::processTileLayer(const nlohmann::json& layer) {
		// 1. Infinitive map 
		if (layer.contains("chunks")) {
			for (const auto& chunk : layer["chunks"]) {
				uint32_t startX = chunk["x"];
				uint32_t startY = chunk["y"];
				uint32_t cWidth = chunk["width"];
				const auto& data = chunk["data"];

				for (uint32_t i = 0; i < data.size(); ++i) {
					uint32_t gid = data[i];
					if (gid == 0) continue; 

					uint32_t tx = (startX + (i % cWidth)) - offsetX;
					uint32_t ty = (startY + (i / cWidth)) - offsetY;

					assignGroundType(tx, ty, gid);
				}
			}
		}
		// 2. Fixed size map
		else if (layer.contains("data")) {
			const auto& data = layer["data"];
			for (uint32_t i = 0; i < data.size(); ++i) {
				uint32_t gid = data[i];
				if (gid == 0) continue;

				uint32_t tx = (i % width);
				uint32_t ty = (i / width);

				assignGroundType(tx, ty, gid);
			}
		}
	}

	void WorldMap::processObjectLayer(const nlohmann::json& layer) {
		for (const auto& obj : layer["objects"]) {
			std::string objName = obj.value("name", "");

			if (!obj.contains("gid")) {
				if (objName == "PlayerSpawn") {
					float sx = (float)obj["x"] - (offsetX * GameConfig::TILE_SIZE);
					float sy = (float)obj["y"] - (offsetY * GameConfig::TILE_SIZE);
					this->setPlayerInitialPosition(sx, sy);
				}
				else if (objName == "Gatekeeper") {
					float nx = (float)obj["x"] - (offsetX * GameConfig::TILE_SIZE);
					float ny = (float)obj["y"] - (offsetY * GameConfig::TILE_SIZE);
					std::string npcName = "World Gatekeeper";
					auto npc = std::make_unique<NPC>(0, npcName, true);
					npc->setPosition(nx, ny);
					this->addNPC(std::move(npc));

					// TODO: add animation here, coz NPC isn't visable from tiled

					std::cout << "[WorldMap] NPC " << npcName << " spawned from Tiled.\n";
				}
			}
			else {
				uint32_t gid = obj.value("gid", 0);
				if (gid == 0) continue;

				float worldX = (float)obj["x"] - (offsetX * GameConfig::TILE_SIZE);
				float correctedTopY = ((float)obj["y"] - (float)obj["height"]) - (offsetY * GameConfig::TILE_SIZE);

				std::string assetName = assetManager.getNameById(gid - 1);
				if (!assetName.empty()) {
					this->placeStructure(worldX, correctedTopY, assetName);
				}
			}
		}
	}

	void WorldMap::saveToFile(const std::string& filename) {
		std::ofstream os(filename, std::ios::binary);
		if (!os.is_open()) return;

		// Save Header & Tiles
		os.write(reinterpret_cast<char*>(&width), sizeof(width));
		os.write(reinterpret_cast<char*>(&height), sizeof(height));
		for (const auto& tile : tiles) {
			os.write(reinterpret_cast<const char*>(&tile.groundType), sizeof(tile.groundType));
		}

		// Save Structures
		uint16_t count = static_cast<uint16_t>(structures.size());
		os.write(reinterpret_cast<char*>(&count), sizeof(count));
		for (auto& s : structures) {
			std::string n = s->getName();
			uint16_t len = static_cast<uint16_t>(n.length());
			os.write(reinterpret_cast<char*>(&len), sizeof(len));
			os.write(n.c_str(), len);

			sf::Vector2f pos = s->getPosition();
			os.write(reinterpret_cast<char*>(&pos.x), sizeof(pos.x));
			os.write(reinterpret_cast<char*>(&pos.y), sizeof(pos.y));
		}

		// NPCs
		uint16_t npcCount = static_cast<uint16_t>(npcs.size());
		os.write(reinterpret_cast<char*>(&npcCount), sizeof(npcCount));
		for (auto& npc : npcs) {
			std::string n = npc->getName();
			uint16_t len = (uint16_t)n.length();
			os.write(reinterpret_cast<char*>(&len), sizeof(len));
			os.write(n.c_str(), len);
			sf::Vector2f pos = npc->getPosition();
			os.write(reinterpret_cast<char*>(&pos), sizeof(pos));
			FactionID f = npc->getTargetFaction(); // TODO: change in the future for getFaction if any changes would occur
			os.write(reinterpret_cast<char*>(&f), sizeof(f));
		}

		os.close();
	}

	void WorldMap::loadMap(const std::string& path) {
		std::ifstream file(path);
		if (!file.is_open()) {
			std::cerr << "[WorldMap] Failed to open TMJ: " << path << std::endl;
			return;
		}

		nlohmann::json tmj;
		file >> tmj;

		int minX = 0, minY = 0, maxX = 0, maxY = 0;
		bool isInfinite = tmj.value("infinite", false);

		if (!isInfinite) {
			maxX = tmj["width"];
			maxY = tmj["height"];
		}
		else {
			bool firstChunk = true;
			for (auto& layer : tmj["layers"]) {
				if (layer["type"] == "tilelayer" && layer.contains("chunks")) {
					for (auto& chunk : layer["chunks"]) {
						int cx = chunk["x"], cy = chunk["y"];
						int cw = chunk["width"], ch = chunk["height"];

						if (firstChunk) {
							minX = cx; minY = cy;
							maxX = cx + cw; maxY = cy + ch;
							firstChunk = false;
						}
						else {
							minX = std::min(minX, cx);
							minY = std::min(minY, cy);
							maxX = std::max(maxX, cx + cw);
							maxY = std::max(maxY, cy + ch);
						}
					}
				}
			}
		}

		this->width = static_cast<uint32_t>(maxX - minX);
		this->height = static_cast<uint32_t>(maxY - minY);
		this->offsetX = minX;
		this->offsetY = minY;

		std::cout << "[WorldMap] Final dimensions: " << width << "x" << height
			<< " Offset: [" << offsetX << "," << offsetY << "]\n";

		this->reshape(width, height);

		for (auto& tile : tiles) tile.release();
		structures.clear();
		npcs.clear();

		// Ground
		for (auto& layer : tmj["layers"]) {
			if (layer["type"] == "tilelayer" && layer["name"] == "Ground") {
				processTileLayer(layer);
			}
		}

		// Objects and obstacles
		for (auto& layer : tmj["layers"]) {
			if (layer["type"] == "objectgroup") {
				processObjectLayer(layer);
			}
		}

	}

	void WorldMap::loadFromFile(const std::string& filename) {
		std::ifstream is(filename, std::ios::binary);
		if (!is.is_open()) return;

		for (auto& tile : tiles) {
			tile.residentObjects.clear();
			tile.release();
		}
		structures.clear();
		npcs.clear();


		is.read(reinterpret_cast<char*>(&width), sizeof(width));
		is.read(reinterpret_cast<char*>(&height), sizeof(height));
		tiles.resize(width * height);

		for (auto& tile : tiles) {
			is.read(reinterpret_cast<char*>(&tile.groundType), sizeof(tile.groundType));
		}

		structures.clear();
		uint16_t count;
		is.read(reinterpret_cast<char*>(&count), sizeof(count));
		for (uint16_t i = 0; i < count; ++i) {
			uint16_t len;
			is.read(reinterpret_cast<char*>(&len), sizeof(len));
			std::string n(len, ' ');
			is.read(&n[0], len);

			sf::Vector2f pos;
			is.read(reinterpret_cast<char*>(&pos.x), sizeof(pos.x));
			is.read(reinterpret_cast<char*>(&pos.y), sizeof(pos.y));

			this->placeStructure(pos.x, pos.y, n);
		}
		is.close();
	}

	void WorldMap::reshape(uint32_t newWidth, uint32_t newHeight) {
		this->width = newWidth;
		this->height = newHeight;

		// Update generation boundaries with margins
		this->width_gen = (width > GameConfig::WORLD_GENERATE_MARGIN) ? width - GameConfig::WORLD_GENERATE_MARGIN : width;
		this->height_gen = (height > GameConfig::WORLD_GENERATE_MARGIN) ? height - GameConfig::WORLD_GENERATE_MARGIN : height;

		structures.clear();
		initializeGrid();
		generateBorders();

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
		if (tile.blocksMovement) return true;

		// 3. Check Objects (Pixel Perfect)
		// Only check the objects, which have blocksMovement True registered to THIS tile.
		for (const auto* obj : tile.residentObjects) {
			if(obj->getDefinition().meta.blocksMovement && obj->getHitbox().contains({ pixelX, pixelY })){
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
		const StructureDefinition& def = assetManager.getDefinition(id);

		// 1. Create Object (on Heap via unique_ptr)
		auto newObj = std::make_unique<WorldObject>(&def, sf::Vector2f(worldX, worldY), assetManager);

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
				if (!at(x, y).canAccept(def.layer)) return false;
			}
		}

		WorldObject* objPtr = newObj.get(); // Keep a raw pointer for the tiles
		// 2. Register to Grid (Spatial Partitioning)

		for (int32_t y = startTy; y <= endTy; ++y) {
			for (int32_t x = startTx; x <= endTx; ++x) {
				auto& tile = at(x, y);

				tile.residentObjects.push_back(objPtr);
				// |= <--> operator OR
				tile.blocksMovement |= def.meta.blocksMovement;
				tile.blocksPlacement |= def.meta.blocksPlacement;
			}
		}

		// 3. Store ownership
		structures.push_back(std::move(newObj));
		return true;
		
	}

	void WorldMap::generateObstacles(float density, uint8_t playerSafeRadius) {
        int32_t spawnX = static_cast<int32_t>(this->spawnPoint.x / GameConfig::TILE_SIZE);
        int32_t spawnY = static_cast<int32_t>(this->spawnPoint.y / GameConfig::TILE_SIZE);
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
		for (int32_t r = 0; r < 5; ++r) {
			for (int32_t dy = -r; dy <= r; ++dy) {
				for (int32_t dx = -r; dx <= r; ++dx) {
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