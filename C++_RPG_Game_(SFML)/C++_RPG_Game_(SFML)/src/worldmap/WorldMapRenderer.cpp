#include "WorldMapRenderer.h"
#include "core/GameConfig.h"


RPG::WorldMapRenderer::WorldMapRenderer() {
    // Default size, will be updated per tile if needed
    tileRect.setSize(sf::Vector2f(static_cast<float>(GameConfig::TILE_SIZE), static_cast<float>(GameConfig::TILE_SIZE)));
}

void RPG::WorldMapRenderer::draw(sf::RenderWindow& window, const WorldMap& worldMap, const AssetManager& assetManager, const SpritesheetManager& sm) {
    // 1. Get the current view for Culling (rendering only what's visible)
    sf::View currentView = window.getView();
    sf::Vector2f center = currentView.getCenter();
    sf::Vector2f size = currentView.getSize();

    // 2. Calculate visible range
    int16_t startX = static_cast<int16_t>((center.x - size.x / 2.0f) / GameConfig::TILE_SIZE);
    int16_t startY = static_cast<int16_t>((center.y - size.y / 2.0f) / GameConfig::TILE_SIZE);
    int16_t endX = static_cast<int16_t>((center.x + size.x / 2.0f) / GameConfig::TILE_SIZE) + 1;
    int16_t endY = static_cast<int16_t>((center.y + size.y / 2.0f) / GameConfig::TILE_SIZE) + 1;

    // Boundary check
    startX = std::max<int16_t>(0, startX);
    startY = std::max<int16_t>(0, startY);
    endX = std::min<int16_t>(worldMap.getWidth(), endX);
    endY = std::min<int16_t>(worldMap.getHeight(), endY);

    for (int16_t y = startY; y < endY; ++y) {
        for (int16_t x = startX; x < endX; ++x) {
            const Tile& tile = worldMap.at(x, y);
            const StructureDefinition& def = assetManager.getDefinition(tile.structure);

            // Skip rendering if there's no texture key (like StructureType::None)
            if (def.textureKey.empty()) continue;

            // 3. Set visual properties based on StructureSize
            // If it's a 2x2 building, the rectangle size becomes (GameConfig::TILE_SIZE * 2)
            float drawWidth = static_cast<float>(def.size.width * GameConfig::TILE_SIZE);
            float drawHeight = static_cast<float>(def.size.height * GameConfig::TILE_SIZE);
            tileRect.setSize(sf::Vector2f(drawWidth, drawHeight));

            // 4. Set position
            tileRect.setPosition(sf::Vector2f(static_cast<float>(x * GameConfig::TILE_SIZE),
                static_cast<float>(y * GameConfig::TILE_SIZE)));

            // 5. Apply Texture
            const sf::Texture* tex = sm.getSpritesheet(def.textureKey);
            if (tex) {
                tileRect.setTexture(tex);
                tileRect.setFillColor(sf::Color::White);
            }
            else {
                // Fallback if texture is missing
                tileRect.setTexture(nullptr);
                tileRect.setFillColor(def.blocksMovement ? sf::Color(100, 100, 100) : sf::Color(50, 150, 50));
            }

            window.draw(tileRect);

            // Reset size back to default for next iterations
            tileRect.setSize(sf::Vector2f(static_cast<float>(GameConfig::TILE_SIZE), static_cast<float>(GameConfig::TILE_SIZE)));
        }
    }
}