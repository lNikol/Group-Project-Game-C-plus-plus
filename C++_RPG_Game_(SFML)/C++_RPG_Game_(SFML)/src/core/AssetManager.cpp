#include "AssetManager.h"

namespace RPG {

	// Spritesheet Handling
	void AssetManager::addSpritesheet(const std::string& name, const std::string& filepath) {
		if (spritesheetMap.count(name) > 0) {
			std::cerr << "[AssetManager] Spritesheet \"" << name << "\" already exists! Aborting.\n";
			return;
		}

		Spritesheet spritesheet;
		if (!spritesheet.loadFromFile(filepath)) {
			std::cerr << "[AssetManager] Failed to load Spritesheet: \"" << filepath << "\"\n";
			return;
		}

		spritesheetMap.emplace(name, std::move(spritesheet));
	}

	const Spritesheet* AssetManager::getSpritesheet(const std::string& name) const {
		auto it = spritesheetMap.find(name);
		return (it != spritesheetMap.end()) ? &it->second : nullptr;
	}


	// Texture Handling
	void AssetManager::addTexture(const std::string& name, const std::string& filepath) {
		if (textureMap.count(name) > 0) {
			std::cerr << "[AssetManager] Texture \"" << name << "\" already exists! Aborting.\n";
			return;
		}

		sf::Texture texture;
		if (!texture.loadFromFile(filepath)) {
			std::cerr << "[AssetManager] Failed to load Texture: \"" << filepath << "\"\n";
			return;
		}

		textureMap.emplace(name, std::move(texture));
	}

	const sf::Texture* AssetManager::getTexture(const std::string& name) const {
		auto it = textureMap.find(name);
		return (it != textureMap.end()) ? &it->second : nullptr;
	}


	// Font Handling
	void AssetManager::addFont(const std::string& name, const std::string& filepath) {
		if (fontMap.count(name) > 0) {
			std::cerr << "[AssetManager] Font \"" << name << "\" already exists! Aborting.\n";
			return;
		}

		sf::Font font;
		if (!font.openFromFile(filepath)) {
			std::cerr << "[AssetManager] Failed to open Font: \"" << filepath << "\"\n";
			return;
		}

		fontMap.emplace(name, std::move(font));
	}

	const sf::Font* AssetManager::getFont(const std::string& name) const {
		auto it = fontMap.find(name);
		return (it != fontMap.end()) ? &it->second : nullptr;
	}

}