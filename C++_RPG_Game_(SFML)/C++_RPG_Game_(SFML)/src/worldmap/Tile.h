#pragma once
#include "StructureDefinition.h"
#include "core/Constants.h"
#include "entities/WorldObject.h"
#include <cstdint>

namespace RPG {

	/**
	 * @brief Represents a 3D coordinate point in the grid system.
	 */
	struct Position {
		int32_t x, y, z;
	};

	/**
	 * @brief A single cell within the WorldMap grid.
	 * * Tiles store information about terrain structures, presence of actors (Player/Monster),
	 * danger levels, and faction ownership.
	 */
	class Tile {
	public:
		StructureType groundType;    // Type of terrain on this tile.
		Position pos;                // Grid coordinates of the tile.
		float speedModifier = 1.f;
		// bool hasPlayer = false;      // True if the player is currently on this tile.
		// bool hasMonster = false;     // True if a hostile entity is present.
		float localDangerLvl = 0.0f; // Procedural or static danger rating.
		GameConfig::Faction faction = GameConfig::Faction::NEUTRAL; // Ownership status.

		bool isWalkable = true;
		bool isOccupied = false;

		std::vector<WorldObject*> residentObjects;
		
		/**
		 * @brief Default constructor initializing an empty neutral tile.
		 */
		Tile();

		/**
		 * @brief Constructs a tile with specific parameters.
		 * @param structure The initial structure type.
		 * @param x X coordinate.
		 * @param y Y coordinate.
		 * @param z Z coordinate (height/layer).
		 */
		Tile(StructureType object, int32_t x, int32_t y, int32_t z);
		
		void release();
		void occupy();


		/**
         * @brief Checks if the tile is clear of obstacles and actors for spawning.
         * @return true If an entity can be placed here.
         */
		bool isAvailableForSpawn() const;

		/**
		 * @brief Checks if the tile is clear of obstacles for spawning.
		 * @return true If an obstacle can be placed here.
		 */
		bool isAvailableForPlace() const;
		/**
		 * @brief Removes the monster flag from the tile.
		 */
		// void clearMonster();
	};

}