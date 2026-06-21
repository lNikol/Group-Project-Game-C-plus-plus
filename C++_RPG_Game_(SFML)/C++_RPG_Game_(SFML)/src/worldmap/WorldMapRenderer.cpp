#include "WorldMapRenderer.h"
#include "core/Constants.h"

namespace RPG {

    WorldMapRenderer::WorldMapRenderer() {
        tileRect.setSize({
            static_cast<float>(GameConfig::TILE_SIZE),
            static_cast<float>(GameConfig::TILE_SIZE)
        });
    }

    void WorldMapRenderer::setupView(sf::RenderWindow& window, const WorldMap& worldMap) {
        if (!worldMap.getPlayer()) return;

        constexpr float zoomFactor = 2.0f;
        sf::Vector2f logicalSize = Window::getLogicalSize(window.getSize());
        logicalSize.x /= zoomFactor;
        logicalSize.y /= zoomFactor;

        sf::View view;
        view.setSize(logicalSize);
        view.setCenter(worldMap.getPlayer()->getPosition());
        window.setView(view);
    }

    void WorldMapRenderer::drawGround(sf::RenderWindow& window, const WorldMap& worldMap) {
        sf::View         view   = window.getView();
        sf::Vector2f     center = view.getCenter();
        sf::Vector2f     size   = view.getSize();

        int32_t startX = std::max<int32_t>(0,
            static_cast<int32_t>((center.x - size.x / 2.f) / GameConfig::TILE_SIZE) - 2);
        int32_t startY = std::max<int32_t>(0,
            static_cast<int32_t>((center.y - size.y / 2.f) / GameConfig::TILE_SIZE) - 2);
        int32_t endX = std::min<int32_t>(static_cast<int32_t>(worldMap.getWidth()),
            static_cast<int32_t>((center.x + size.x / 2.f) / GameConfig::TILE_SIZE) + 3);
        int32_t endY = std::min<int32_t>(static_cast<int32_t>(worldMap.getHeight()),
            static_cast<int32_t>((center.y + size.y / 2.f) / GameConfig::TILE_SIZE) + 3);

        const AssetManager& am    = AssetManager::getInstance();
        const sf::Texture*  atlas = am.getTexture("world_atlas");
        if (!atlas) return;

        sf::Sprite sprite(*atlas);

        for (int32_t y = startY; y < endY; ++y) {
            for (int32_t x = startX; x < endX; ++x) {
                const Tile& tile = worldMap.at(x, y);
                const auto& def  = am.getDefinition(am.getDefinitionByType(tile.groundType));

                sprite.setTextureRect(sf::IntRect(
                    { static_cast<int>(def.textureStartPos.x),
                      static_cast<int>(def.textureStartPos.y) },
                    { static_cast<int>(def.size.x),
                      static_cast<int>(def.size.y) }
                ));
                sprite.setPosition({
                    static_cast<float>(x * GameConfig::TILE_SIZE),
                    static_cast<float>(y * GameConfig::TILE_SIZE)
                });
                window.draw(sprite);
            }
        }
    }

    void WorldMapRenderer::drawDebugHUD(sf::RenderWindow& window, const WorldMap& worldMap) {
        if (!worldMap.getPlayer()) return;

        sf::View savedView = window.getView();
        sf::View debugView = sf::View(sf::FloatRect({0.f, 0.f}, Window::getLogicalSize(window.getSize())));
        window.setView(debugView);

        const sf::Font* font = AssetManager::getInstance().getFont("PixelFont");
        if (font) {
            sf::Text text(*font);
            auto pos = worldMap.getPlayer()->getPosition();
            text.setString(
                "X: " + std::to_string(static_cast<int>(pos.x / GameConfig::TILE_SIZE)) +
                "  Y: " + std::to_string(static_cast<int>(pos.y / GameConfig::TILE_SIZE))
            );
            text.setCharacterSize(24);
            text.setFillColor(sf::Color::White);
            text.setOutlineColor(sf::Color::Black);
            text.setOutlineThickness(1.f);
            text.setPosition({ 10.f, 10.f });
            window.draw(text);
        }

        window.setView(savedView);
    }

}
