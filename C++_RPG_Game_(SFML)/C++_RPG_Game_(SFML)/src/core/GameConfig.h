#pragma once
#include <string>

namespace RPG {
	namespace GameConfig {
		inline const std::string ASSETS_PATH = "assets/";
		inline const std::string TEXTURES_PATH = ASSETS_PATH + "textures/";
		enum class Faction { NEUTRAL, LIGHT, DARK };
		const uint8_t TILE_SIZE = 32;
	}
}