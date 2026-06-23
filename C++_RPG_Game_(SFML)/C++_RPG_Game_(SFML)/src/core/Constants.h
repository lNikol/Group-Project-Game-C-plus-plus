#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <string>

namespace RPG {

	using Spritesheet = sf::Texture;

	namespace Window {
		const uint16_t WIDTH = 1200;
		const uint16_t HEIGHT = 800;
		const std::string TITLE = "RPG GAME";
		const uint8_t BASE_FPS = 60;

		inline sf::Vector2f getLogicalSize(const sf::Vector2u& winSize) {
			float logicalHeight = HEIGHT;
			float logicalWidth = HEIGHT * (winSize.x / (float)winSize.y);
			return {logicalWidth, logicalHeight};
		}
	}

	namespace GameConfig {
		inline const std::string ASSETS_PATH = "assets/";
		inline const std::string JSON_PATH = ASSETS_PATH + "jsons/";
		inline const std::string TEXTURES_PATH = ASSETS_PATH + "textures/";
		inline const std::string ANIMATIONS_PATH = ASSETS_PATH + "animations/";
		inline const std::string PROCESSED_PATH = ASSETS_PATH + "assets_processed/";
		inline const std::string WORLD_PATH = PROCESSED_PATH + "main_world/";
		inline const std::string FACTION_PATH = PROCESSED_PATH + "faction/";

		inline const std::string SAVES_PATH = "temp/saves/";

		enum class Faction { NEUTRAL, LIGHT, DARK };
		const uint8_t TILE_SIZE = 32;
		const uint16_t WORLD_WIDTH = 50;
		const uint16_t WORLD_HEIGHT = 50;
		const uint16_t WORLD_GENERATE_MARGIN = 2;
	}
}