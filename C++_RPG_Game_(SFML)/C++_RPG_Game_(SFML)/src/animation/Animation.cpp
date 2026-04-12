#include "Animation.h"
#include "AnimationBuilder.h"

namespace RPG {

    Animation::Animation() : 
        spritesheet(nullptr), 
        frameStartPos(), 
        frameSize({GameConfig::TILE_SIZE, GameConfig::TILE_SIZE}),
        frameCount(0), 
        frameGap(0), 
        looped(true), 
        currentFrame(),
        currentFrameIndex(0), 
        finished(false), 
        timer(0.0), 
        frameDuration(0.1)
    {
    }

    Animation::~Animation() {
    }

    AnimationBuilder Animation::builder() {
        return AnimationBuilder();
    }

    void Animation::update(float dt) {
        if (finished) return;

        // std::cout << "Frame: " << (int)currentFrameIndex << "\n";


        timer += dt;
        if (timer >= frameDuration) {

            timer -= frameDuration;
            currentFrameIndex++;

            
            if (currentFrameIndex >= frameCount) {
                if (looped) {
                    currentFrameIndex = 0;
                }
                else {
                    currentFrameIndex = frameCount - 1;
                    finished = true;
                }
            }
        }

        currentFrame.position.x = frameStartPos.x + (currentFrameIndex * (frameSize.x + frameGap));
        currentFrame.position.y = frameStartPos.y;
        currentFrame.size.x = frameSize.x;
        currentFrame.size.y = frameSize.y;
    }
        
    void Animation::reset() {
        timer = 0.0;
        currentFrameIndex = 0;
        finished = false;
        currentFrame.position.x = frameStartPos.x;
        currentFrame.position.y = frameStartPos.y;
    }

    const sf::IntRect& Animation::getCurrentFrame() const {
        return currentFrame;
    }

    const Spritesheet* Animation::getSpritesheet() const {
        return spritesheet;
    }

    bool Animation::isFinished() const {
        return finished;
    }

    const uint8_t Animation::getCurrentFrameIndex() const {
        return currentFrameIndex;
    }

}