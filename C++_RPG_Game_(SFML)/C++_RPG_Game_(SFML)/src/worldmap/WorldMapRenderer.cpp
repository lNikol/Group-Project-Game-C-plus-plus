#include "WorldMapRenderer.h"
#include "core/GameConfig.h"

namespace RPG {

    WorldMapRenderer::WorldMapRenderer() {
        // Default size, will be updated per tile if needed
        tileRect.setSize(sf::Vector2f(static_cast<float>(GameConfig::TILE_SIZE), static_cast<float>(GameConfig::TILE_SIZE)));
    }

    void WorldMapRenderer::draw(sf::RenderWindow& window, const WorldMap& worldMap, const AssetManager& assetManager, const SpritesheetManager& sm, float percentView) {
        // 1. Get the current view for Culling (rendering only what's visible)
        sf::View currentView = window.getView();
        sf::Vector2f center = currentView.getCenter();
        sf::Vector2f size = currentView.getSize();

        bool isFogActive = (percentView < 1.0f);
        float maxVisibleDist = (size.x / 2.0f) * percentView; // X% of view
        float fadeRange = 150.0f;


        // 2. Calculate visible range & boundary check
        int16_t startX = std::max<int16_t>(0, static_cast<int16_t>((center.x - size.x / 2.0f) / GameConfig::TILE_SIZE));
        int16_t startY = std::max<int16_t>(0, static_cast<int16_t>((center.y - size.y / 2.0f) / GameConfig::TILE_SIZE));

        int16_t endX = std::min<int16_t>(worldMap.getWidth(), static_cast<int16_t>((center.x + size.x / 2.0f) / GameConfig::TILE_SIZE) + 1);
        int16_t endY = std::min<int16_t>(worldMap.getHeight(), static_cast<int16_t>((center.y + size.y / 2.0f) / GameConfig::TILE_SIZE) + 1);

        for (int16_t y = startY; y < endY; ++y) {
            for (int16_t x = startX; x < endX; ++x) {
                const Tile& tile = worldMap.at(x, y);
                const StructureDefinition& def = assetManager.getDefinition(tile.structure);

                // Skip rendering if there's no texture key (like StructureType::None)
                if (def.textureKey.empty()) continue;

                // 3. Set visual properties based on StructureSize
                // TODO: check if it wroks
                // If it's a 2x2 building, the rectangle size becomes (GameConfig::TILE_SIZE * 2)
                float drawWidth = static_cast<float>(def.size.width * GameConfig::TILE_SIZE);
                float drawHeight = static_cast<float>(def.size.height * GameConfig::TILE_SIZE);
                tileRect.setSize(sf::Vector2f(drawWidth, drawHeight));

                // 4. Set position
                sf::Vector2f tilePos(static_cast<float>(x * GameConfig::TILE_SIZE),
                    static_cast<float>(y * GameConfig::TILE_SIZE));

                tileRect.setPosition(tilePos);

                uint8_t alpha = 255;

                if (isFogActive) {
                    float dx = (tilePos.x + (drawWidth / 2.0f)) - center.x;
                    float dy = (tilePos.y + (drawHeight / 2.0f)) - center.y;
                    float distSq = dx * dx + dy * dy; 

                    float maxDistSq = maxVisibleDist * maxVisibleDist;
                    float totalRangeSq = (maxVisibleDist + fadeRange) * (maxVisibleDist + fadeRange);

                    if (distSq > totalRangeSq) {
                        continue; // Outside of visible view
                    }
                    else if (distSq > maxDistSq) {
                        // inside of Fade zone
                        float distance = std::sqrt(distSq);
                        float fadeFactor = (distance - maxVisibleDist) / fadeRange;
                        alpha = static_cast<uint8_t>(255.0f * (1.0f - std::clamp(fadeFactor, 0.0f, 1.0f)));
                    }
                }

                // 5. Rendering
                const sf::Texture* tex = sm.getSpritesheet(def.textureKey);
                tileRect.setTexture(tex);

                sf::Color finalColor = (tex) ? sf::Color::White : (def.blocksMovement ? sf::Color(100, 100, 100) : sf::Color(50, 150, 50));
                finalColor.a = alpha;
                tileRect.setFillColor(finalColor);

                window.draw(tileRect);
            }
        }
    }
}