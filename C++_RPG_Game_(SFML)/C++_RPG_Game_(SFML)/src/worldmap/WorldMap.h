#pragma once
#include "entities/NPC.h"
#include "DifficultySettings.h"
#include "AssetManager.h"
#include "Tile.h"
#include "entities/WorldObject.h"
#include <memory>
#include <stdexcept>
#include <vector>

namespace RPG {

	class WorldMap {
	private:
		uint32_t width, width_gen, height, height_gen;
		std::vector<Tile> tiles; // Only holds ground
		std::vector<std::unique_ptr<WorldObject>> structures; // Holds structues (trees, houses, rocks)
		std::vector<std::unique_ptr<NPC>> npcs; // Holds active npcs
	
	public:
		WorldMap(uint32_t w, uint32_t h);

		uint32_t getWidth() const;
		uint32_t getHeight() const;


		/**
		 * @brief Checks if a given coordinate is within the internal spawnable area.
		 * * This function defines a "safe rectangle" by subtracting a margin from all map edges.
		 * It is used to prevent monsters from spawning too close to the player's starting point
		 * or map boundaries.
		 * * @param x The X-coordinate of the tile.
		 * @param y The Y-coordinate of the tile.
		 * @param margin The distance from the map edge (number of tiles) to be excluded.
		 * @return true if (x,y) is inside the spawn zone, false if it's within the margin or out of bounds.
		 */
		bool isInsideSpawnZone(int32_t x, int32_t y, uint8_t margin) const;
		
		/**
		 * @brief Accesses a tile at specific coordinates by converting 2D to 1D index.
		 * @param x X coordinate
		 * @param y Y coordinate
		 * @return Reference to the Tile at the given position
		 */
		Tile& at(int32_t x, int32_t y);
		const Tile& at(int32_t x, int32_t y) const;
		
		/**
		 * @brief Checks if a pixel position is blocked by a structure.
		 * @param pixelX X coordinate in pixels.
		 * @param pixelY Y coordinate in pixels.
		 * @return true if movement is blocked.
		 */
		bool isBlockingAtPixel(float pixelX, float pixelY) const;

		void refreshMonsters(const DifficultyLevel& level, uint8_t mapEdgeOffset = 5);

		float getTileSpeedModifier(int32_t tx, int32_t ty) const;


		/**
		 * @brief Safely places a multi-tile structure on the map
		 * @param startX X coordinate of the top-left corner
		 * @param startY Y coordinate of the top-left corner
		 * @param type The type of structure to place
		 * @return true if placement was successful
		 */
		bool placeStructure(float worldX, float worldY, const std::string& name);
		bool placeStructureAtTile(int32_t tileX, int32_t tileY, const std::string& name);

		void generateObstacles(float density, const sf::Vector2f& playerStartPos, uint8_t playerSafeRadius = 2);

		sf::Vector2f findNearestSafeTile(sf::Vector2f startPos);

		/**
		 * @brief Adds a new NPC to this specific map instance.
		 * * The NPC is stored as a unique_ptr, meaning the map takes full ownership
		 * of the NPC's lifecycle.
		 * * @param npc A unique_ptr to the NPC object.
		 */
		void addNPC(std::unique_ptr<NPC> npc);
		const std::vector<std::unique_ptr<NPC>>& getNPCs() const;
		std::vector<std::unique_ptr<NPC>>& getNPCs();

		const std::vector<std::unique_ptr<WorldObject>>& getStructures() const;

		void toggleDebugHitbox();
	};

}
