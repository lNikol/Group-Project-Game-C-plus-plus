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

    const std::optional<Animation> AnimationManager::getAnimation(const std::string& name) {
        auto it = animationMap.find(name);

        if (it == animationMap.end()) {
            std::cerr << "[Warning] Animation not found: " << name << "\n";
            return std::nullopt;
        }   

        return it->second;
    }

}