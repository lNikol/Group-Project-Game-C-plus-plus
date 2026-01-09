#pragma once
#include "DifficultySettings.h"
#include "AssetManager.h"
#include "Tile.h"
#include <stdexcept>
#include <vector>

namespace RPG {

	class WorldMap {
	private:
		uint16_t width, height;
		std::vector<Tile> tiles;
		AssetManager& assetManager;
	
	public:
		WorldMap(uint16_t w, uint16_t h, AssetManager& am);

		uint16_t getWidth() const;
		uint16_t getHeight() const;

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
		bool isInsideSpawnZone(int16_t x, int16_t y, uint8_t margin) const;
		
		/**
		 * @brief Accesses a tile at specific coordinates by converting 2D to 1D index.
		 * @param x X coordinate
		 * @param y Y coordinate
		 * @return Reference to the Tile at the given position
		 */
		Tile& at(int16_t x, int16_t y);
		const Tile& at(int16_t x, int16_t y) const;
		
		/**
		 * @brief Checks if a pixel position is blocked by a structure.
		 * @param pixelX X coordinate in pixels.
		 * @param pixelY Y coordinate in pixels.
		 * @return true if movement is blocked.
		 */
		bool isBlockingAtPixel(float pixelX, float pixelY) const;

		void refreshMonsters(const DifficultyLevel& level, uint8_t mapEdgeOffset = 5);

		float getTileSpeedModifier(int16_t tx, int16_t ty) const;


		/**
		 * @brief Safely places a multi-tile structure on the map
		 * @param startX X coordinate of the top-left corner
		 * @param startY Y coordinate of the top-left corner
		 * @param type The type of structure to place
		 * @return true if placement was successful
		 */
		bool placeStructure(int16_t startX, int16_t startY, StructureType type);


		void generateObstacles(float density, const sf::Vector2f& playerStartPos, uint8_t playerSafeRadius = 2);
	};

}