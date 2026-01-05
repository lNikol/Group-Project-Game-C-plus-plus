#pragma once
#include <unordered_map>
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "Constants.h"

namespace RPG {

	class AssetManager {
	private:
		// Storage
		std::unordered_map<std::string, Spritesheet> spritesheetMap;
		std::unordered_map<std::string, sf::Texture> textureMap;
		std::unordered_map<std::string, sf::Font> fontMap;

	public:
		// Spritesheets
		void addSpritesheet(const std::string& name, const std::string& filepath);
		const Spritesheet* getSpritesheet(const std::string& name) const;

		// Textures
		void addTexture(const std::string& name, const std::string& filepath);
		const sf::Texture* getTexture(const std::string& name) const;

		// Fonts
		void addFont(const std::string& name, const std::string& filepath);
		const sf::Font* getFont(const std::string& name) const;
	};

}