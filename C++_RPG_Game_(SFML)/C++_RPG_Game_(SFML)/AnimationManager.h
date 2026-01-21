#pragma once
#include <string>
#include <unordered_map>
#include <iostream>
#include "Animation.h"

namespace RPG {

	class AnimationManager {
		std::unordered_map<std::string, Animation> animationMap;

	public:
		void addAnimation(const std::string& name, Animation animation);
		const Animation* getAnimation(const std::string& name);
	};

}

