#pragma once
#include "entities/NPC.h"
#include "DifficultySettings.h"
#include "AssetManager.h"
#include "Tile.h"
#include "entities/WorldObject.h"
#include <nlohmann/json.hpp>
#include <memory>
#include <stdexcept>
#include <vector>

namespace RPG {

	class WorldMap {
	private:
		uint32_t width, width_gen, height, height_gen;
		sf::Vector2f spawnPoint = { 50,50 };
		bool hasSpawnPoint = false;
		int32_t offsetX = 0, offsetY = 0;
		AssetManager& assetManager;
		
		std::vector<Tile> tiles; // Only holds ground
		std::vector<std::unique_ptr<WorldObject>> structures; // Holds structues (trees, houses, rocks)
		std::vector<std::unique_ptr<NPC>> npcs; // Holds active npcs
	
	public:
		WorldMap(uint32_t w, uint32_t h, AssetManager& am);

		uint32_t getWidth() const;
		uint32_t getHeight() const;
		sf::Vector2f getSpawnPoint() const;
		bool getHasSpawnPoint() const;

		void setPlayerInitialPosition(float x, float y);
		/**
		 * @brief Initialize grid with default values
		 */
		void initializeGrid();
		
		/**
		 * @brief Generate borders for map
		 */
		void generateBorders();

		/**
		 * @brief Safely assigns a ground type to a specific tile based on its GID.
		 * * Checks if the calculated coordinates are within the current map boundaries.
		 * Fetches the asset definition from the AssetManager using the GID (minus 1).
		 * * @param tx Local X coordinate in the tiles array (offset-corrected).
		 * @param ty Local Y coordinate in the tiles array (offset-corrected).
		 * @param gid The Global ID (GID) provided by Tiled.
		 */
		void assignGroundType(uint32_t tx, uint32_t ty, uint32_t gid);

		/**
		 * @brief Processes tile layers from the TMJ (Tiled JSON) format.
		 * * This function handles both fixed-size and infinite (chunked) maps.
		 * It is responsible for setting the groundType for every tile in the grid.
		 * * @param layer The JSON object representing a "tilelayer".
		 */
		void processTileLayer(const nlohmann::json& layer);

		/**
		 * @brief Processes object groups from the TMJ (Tiled JSON) format.
		 * * Reads graphical objects (e.g., trees, chests, flowers) and attempts to
		 * place them in the world using placeStructure. It automatically corrects
		 * the coordinate system discrepancy between Tiled (Bottom-Left) and
		 * SFML (Top-Left).
		 * * @param layer The JSON object representing an "objectgroup".
		 */
		void processObjectLayer(const nlohmann::json& layer);
		


		/**
		 * @brief Loads a Tiled Map Editor file (.tmj) in JSON format.
		 * * Supports infinite maps (chunks) and object layers for obstacles.
		 * @param path Path to the .tmj file.
		 */
		void loadMap(const std::string& path);

		/**
		 * @brief Serializes the current map state to a binary file.
		 * * Saves dimensions, tile types, and all placed structures
		 * @param filename Destination path for the binary data.
		 */
		void saveToFile(const std::string& filename);

		/**
		* @brief Deserializes the map state from a binary file and restores all entities.
		* @param filename Path to the .bin file.
		 */
		void loadFromFile(const std::string& filename);


		/**
		 * @brief Safely changes the map dimensions and reinitializes the tile grid.
		 * @param newWidth New width in tiles.
		 * @param newHeight New height in tiles.
		 * @note This will clear all existing tile data but keep structures/NPCs
		 * unless they fall outside the new boundaries.
		 */
		void reshape(uint32_t newWidth, uint32_t newHeight);

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
		 * @brief Checks if a specific pixel coordinate is blocked by terrain or objects.
		 * * First checks global tile properties (like water), then iterates through
		 * resident objects that have the 'blocksMovement' metadata enabled.
		 * Uses the precise hitbox calculated during asset processing.
		 * * @param pixelX World X coordinate in pixels.
		 * @param pixelY World Y coordinate in pixels.
		 * @return true If the position is impassable.
		 * @return false If the position is clear for movement.
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

		void generateObstacles(float density, uint8_t playerSafeRadius = 2);

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
