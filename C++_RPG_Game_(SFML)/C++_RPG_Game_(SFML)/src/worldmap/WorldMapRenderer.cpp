#include "WorldMapRenderer.h"
#include "core/GameConfig.h"

namespace RPG {

    WorldMapRenderer::WorldMapRenderer() {
        // Default size, will be updated per tile if needed
        tileRect.setSize(sf::Vector2f(static_cast<float>(GameConfig::TILE_SIZE), static_cast<float>(GameConfig::TILE_SIZE)));
    }

    void WorldMapRenderer::draw(
        sf::RenderWindow& window,
        const WorldMap& worldMap,
        const AssetManager& assetManager,
        float percentView
    ) {
        // 1. View & Culling Calculations
        sf::View currentView = window.getView();
        sf::Vector2f center = currentView.getCenter();
        sf::Vector2f size = currentView.getSize();

        bool isFogActive = (percentView < 1.0f);
        float maxVisibleDist = (size.x / 2.0f) * percentView;
        float fadeRange = 150.0f;

        // 2. Bounds Calculation (Optimization)
        int16_t startX = std::max<int16_t>(0, static_cast<int16_t>((center.x - size.x / 2.0f) / GameConfig::TILE_SIZE));
        int16_t startY = std::max<int16_t>(0, static_cast<int16_t>((center.y - size.y / 2.0f) / GameConfig::TILE_SIZE));

        int16_t endX = std::min<int16_t>(worldMap.getWidth(), static_cast<int16_t>((center.x + size.x / 2.0f) / GameConfig::TILE_SIZE) + 1);
        int16_t endY = std::min<int16_t>(worldMap.getHeight(), static_cast<int16_t>((center.y + size.y / 2.0f) / GameConfig::TILE_SIZE) + 1);

        // 3. Render Loop
        for (int16_t y = startY; y < endY; ++y) {
            for (int16_t x = startX; x < endX; ++x) {

                const Tile& tile = worldMap.at(x, y);

                // --- CALC POSITION & FOG ---
                float drawX = static_cast<float>(x * GameConfig::TILE_SIZE);
                float drawY = static_cast<float>(y * GameConfig::TILE_SIZE);
                sf::Vector2f tilePos(drawX, drawY);

                uint8_t alpha = 255;
                if (isFogActive) {
                    float dx = (drawX + GameConfig::TILE_SIZE / 2.0f) - center.x;
                    float dy = (drawY + GameConfig::TILE_SIZE / 2.0f) - center.y;
                    float distSq = dx * dx + dy * dy;

                    float maxDistSq = maxVisibleDist * maxVisibleDist;
                    float totalRangeSq = (maxVisibleDist + fadeRange) * (maxVisibleDist + fadeRange);

                    if (distSq > totalRangeSq) continue; // Culling
                    else if (distSq > maxDistSq) {
                        float distance = std::sqrt(distSq);
                        float fadeFactor = (distance - maxVisibleDist) / fadeRange;
                        alpha = static_cast<uint8_t>(255.0f * (1.0f - std::clamp(fadeFactor, 0.0f, 1.0f)));
                    }
                }

                // --- LAYER 0: GROUND (Always Draw) ---
                const StructureDefinition& groundDef = assetManager.getDefinition(tile.groundType);
                const sf::Texture* groundTex = assetManager.getSpritesheet(groundDef.textureKey);

                if (groundTex) {
                    tileRect.setSize(sf::Vector2f(static_cast<float>(GameConfig::TILE_SIZE), static_cast<float>(GameConfig::TILE_SIZE)));
                    tileRect.setPosition(tilePos);
                    tileRect.setTexture(groundTex);
                    tileRect.setFillColor(sf::Color(255, 255, 255, alpha));
                    tileRect.setOutlineThickness(0);
                    window.draw(tileRect);
                }

                // --- LAYER 1: OBJECT (Draw on top) ---
                if (tile.objectType != StructureType::None) {
                    const StructureDefinition& objDef = assetManager.getDefinition(tile.objectType);

                    // Only draw if it's a visual object (has a texture key)
                    if (!objDef.textureKey.empty()) {
                        const sf::Texture* objTex = assetManager.getSpritesheet(objDef.textureKey);

                        if (objTex) {
                            float objW = static_cast<float>(objDef.size.width * GameConfig::TILE_SIZE);
                            float objH = static_cast<float>(objDef.size.height * GameConfig::TILE_SIZE);

                            tileRect.setSize(sf::Vector2f(objW, objH));
                            tileRect.setPosition(tilePos);
                            tileRect.setTexture(objTex);
                            tileRect.setFillColor(sf::Color(255, 255, 255, alpha));
                            window.draw(tileRect);
                        }
                    }
                }
            }
        }
    }
}