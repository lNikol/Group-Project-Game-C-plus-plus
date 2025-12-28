#include "core/SpritesheetManager.h"

namespace RPG {

	void SpritesheetManager::addSpritesheet(const std::string& name, const std::string& filepath) {

		if (spritesheetMap.count(name) > 0) {
			std::cerr << "Spritesheet with this name \"" << filepath << "\" already exists!\n";
			std::cerr << "Aborting...\n";
			return;
		}

		Spritesheet spritesheet;
		if (!spritesheet.loadFromFile(filepath)) {
			std::cerr << "Could not load the texture from: \"" << filepath << "\"!\n";
			std::cerr << "Aborting...\n";
			return;
		}

		spritesheetMap.emplace(name, std::move(spritesheet));
	}

	const Spritesheet* SpritesheetManager::getSpritesheet(const std::string& name) const {
		auto it = spritesheetMap.find(name);
		if (it == spritesheetMap.end()) {
			return nullptr;
		}

		return &it->second;
	}

}