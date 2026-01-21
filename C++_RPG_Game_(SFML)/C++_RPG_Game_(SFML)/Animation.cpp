#include "Animation.h"
#include "AnimationBuilder.h"

namespace RPG {

    Animation::Animation()
        : spritesheet(nullptr), frameStartPos(), frameSize(),
          frameCount(0), frameGapPx(0), looped(false), currentFrame()
    {
    }

    AnimationBuilder Animation::builder() {
        return AnimationBuilder();
    }

    void Animation::update(float dt) {
        
    }

    const sf::IntRect& Animation::getCurrentFrame() const {
        return currentFrame;
    }

}