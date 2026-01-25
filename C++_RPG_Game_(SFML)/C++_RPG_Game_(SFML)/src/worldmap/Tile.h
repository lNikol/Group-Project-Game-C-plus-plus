#pragma once
#include "StructureDefinition.h"
#include "core/Constants.h"
#include <cstdint>

namespace RPG {

	/**
	 * @brief Represents a 3D coordinate point in the grid system.
	 */
	struct Position {
		int16_t x, y, z;
	};

	/**
	 * @brief A single cell within the WorldMap grid.
	 * * Tiles store information about terrain structures, presence of actors (Player/Monster),
	 * danger levels, and faction ownership.
	 */
	class Tile {
	public:
		StructureType groundType;    // Type of terrain on this tile.
		StructureType objectType;    // Type of structure/object on this tile
		Position pos;                // Grid coordinates of the tile.
		bool hasPlayer = false;      // True if the player is currently on this tile.
		bool hasMonster = false;     // True if a hostile entity is present.
		float localDangerLvl = 0.0f; // Procedural or static danger rating.
		GameConfig::Faction faction = GameConfig::Faction::NEUTRAL; // Ownership status.
		
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
		Tile(StructureType ground, StructureType object, int16_t x, int16_t y, int16_t z);
		
		/**
         * @brief Checks if the tile is clear of obstacles and actors for spawning.
         * @return true If an entity can be placed here.
         */
		bool isAvailableForSpawn() const;

		/**
		 * @brief Removes the monster flag from the tile.
		 */
		void clearMonster();
	};

}