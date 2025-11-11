#pragma once
#include <unordered_map>
#include <string>
#include <iostream>
#include "Constants.h"

namespace RPG {

	class SpritesheetManager {
		std::unordered_map<std::string, Spritesheet> spritesheetMap;

	public:
		void addSpritesheet(const std::string& name, const std::string& filepath);
		const Spritesheet* getSpritesheet(const std::string& name) const;
	};

}

