#include "AnimationManager.h"

namespace RPG {

    void AnimationManager::addAnimation(const std::string& name, Animation animation) {

        auto [it, ok] = animationMap.try_emplace(name, std::move(animation));

        if (!ok) {
            std::cerr << "Spritesheet with this name \"" << name << "\" already exists!\n";
            std::cerr << "Aborting...\n";
            return;
        }
        
    }

    const Animation* AnimationManager::getAnimation(const std::string& name) {
        auto it = animationMap.find(name);

        if (it == animationMap.end()) {
            return nullptr;
        }

        return &it->second;
    }

}