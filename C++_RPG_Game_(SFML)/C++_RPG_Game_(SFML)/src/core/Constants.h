#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <string>

namespace RPG {

	using Spritesheet = sf::Texture;

	namespace Window {
		const uint16_t WIDTH = 800;
		const uint16_t HEIGHT = 600;
		const std::string TITLE = "RPG GAME";
		const uint8_t BASE_FPS = 60;
	}

	namespace GameConfig {
		inline const std::string ASSETS_PATH = "assets/";
		inline const std::string TEXTURES_PATH = ASSETS_PATH + "textures/";
		enum class Faction { NEUTRAL, LIGHT, DARK };
		const uint8_t TILE_SIZE = 32;
	}
}