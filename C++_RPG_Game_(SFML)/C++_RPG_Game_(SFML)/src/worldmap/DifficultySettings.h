#pragma once
#include <string>
#include <cstdint>

namespace RPG {

	struct DifficultyLevel {
		std::string name;
		int8_t spawnChance; // in %
		float dangerLvl; // Monster power multiplier
		uint8_t maxMonsters;
	};
	namespace GameConfig {
		const DifficultyLevel DAY = { "Day", 15, 1.0f, 20 };
		const DifficultyLevel EVENING = { "Evening", 35, 1.15f, 30 };
		const DifficultyLevel NIGHT = { "Night", 75, 1.75f, 40 };
	}
}