#include "AnimationLoader.h"

namespace RPG {

	inline const sf::Vector2u DEFAULT_START_POS = { 0,0 };
	inline const sf::Vector2u DEFAULT_FRAME_SIZE = { GameConfig::TILE_SIZE, GameConfig::TILE_SIZE };
	const uint8_t DEFAULT_FRAME_COUNT = 0;
	const uint8_t DEFAULT_FRAME_GAP = 0;
	const bool DEFAULT_LOOPED = true;
	const float DEFAULT_FRAME_DURATION = .1f;

    AnimationMap AnimationLoader::loadAnimations(const std::string& filepath, const Spritesheet* spritesheet) {

        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open animation file: " << filepath << "\n";
            return {};
        }

        json data;
        try {
            file >> data;
        }
        catch (const json::parse_error& e) {
            std::cerr << "JSON Parse Error in " << filepath << ": " << e.what() << "\n";
            return {};
        }

        AnimationMap animations;

        auto getVec2u = [](const json& j, const std::string& key, const sf::Vector2u& fallback) -> sf::Vector2u {
            if (j.contains(key) && j[key].is_array() && j[key].size() >= 2) {
                return { j[key][0].get<unsigned int>(), j[key][1].get<unsigned int>() };
            }
            return fallback;
        };

        sf::Vector2u rootStartPos = getVec2u(data, "defaultStartPos", DEFAULT_START_POS);
        sf::Vector2u rootFrameSize = getVec2u(data, "defaultFrameSize", DEFAULT_FRAME_SIZE);
        uint8_t rootFrameCount = data.value("defaultFrameCount", DEFAULT_FRAME_COUNT);
        uint8_t rootFrameGap = data.value("defaultFrameGap", DEFAULT_FRAME_GAP);
        bool rootLooped = data.value("defaultLooped", DEFAULT_LOOPED);
        float rootFrameDuration = data.value("defaultFrameDuration", DEFAULT_FRAME_DURATION);

        if (data.contains("animations")) {
            for (const auto& [animName, animData] : data["animations"].items()) {

                sf::Vector2u startPos = getVec2u(animData, "startPos", rootStartPos);
                sf::Vector2u frameSize = getVec2u(animData, "frameSize", rootFrameSize);
                uint8_t frameCount = animData.value("frameCount", rootFrameCount);
                uint8_t frameGap = animData.value("frameGap", rootFrameGap);
                bool looped = animData.value("looped", rootLooped);
                float frameDuration = animData.value("frameDuration", rootFrameDuration);

                animations[animName] = Animation::builder()
                    .spritesheet(spritesheet)
                    .frameStartPos(sf::Vector2u(static_cast<unsigned int>(startPos.x), static_cast<unsigned int>(startPos.y)))
                    .frameSize(frameSize)
                    .frameCount(frameCount)
                    .frameGap(frameGap)
                    .looped(looped)
                    .frameDuration(frameDuration)
                    .build();
            }
        }

        return animations;
    }

}

