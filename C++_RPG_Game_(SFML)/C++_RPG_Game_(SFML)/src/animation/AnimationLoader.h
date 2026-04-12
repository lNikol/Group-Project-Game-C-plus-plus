#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include "animation/AnimationBuilder.h"
#include "animation/Animation.h"
#include "core/Constants.h"

using json = nlohmann::json;
using AnimationMap = std::map<std::string, RPG::Animation>;

namespace RPG {
	class AnimationLoader {
	public:
		static AnimationMap loadAnimations(const std::string& filepath, const Spritesheet* spritesheet);
	};
}