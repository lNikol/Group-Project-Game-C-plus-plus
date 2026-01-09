#pragma once
#include "StructureDefinition.h"
#include "core/Constants.h"
#include <cstdint>

namespace RPG {

	struct Position {
		int16_t x, y, z;
	};

	class Tile {
	public:
		StructureType structure;
		Position pos;
		bool hasPlayer = false;
		bool hasMonster = false;
		float localDangerLvl = 0.0f;
		GameConfig::Faction faction = GameConfig::Faction::NEUTRAL;

		Tile();
		Tile(StructureType structure, int16_t x, int16_t y, int16_t z);
		bool isAvailableForSpawn() const;
		void clearMonster();
	};

}