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
        const Player& player,
        float precentView
    ) {
        // --- 1. DRAW GROUND (Layer 0) ---
        // Ground is always behind everything, so just draw it first. No sorting needed.

        sf::View currentView = window.getView();
        sf::Vector2f center = currentView.getCenter();
        sf::Vector2f size = currentView.getSize();

        // Calculate visible range to avoid drawing the whole world
        int32_t startX = std::max<int32_t>(0, (center.x - size.x / 2) / GameConfig::TILE_SIZE);
        int32_t startY = std::max<int32_t>(0, (center.y - size.y / 2) / GameConfig::TILE_SIZE);
        int32_t endX = std::min<int32_t>(worldMap.getWidth(), (center.x + size.x / 2) / GameConfig::TILE_SIZE + 1);
        int32_t endY = std::min<int32_t>(worldMap.getHeight(), (center.y + size.y / 2) / GameConfig::TILE_SIZE + 1);

        for (int32_t y = startY; y < endY; ++y) {
            for (int32_t x = startX; x < endX; ++x) {
                const Tile& tile = worldMap.at(x, y);

                const sf::Texture* tex = assetManager.getTexture("grass");
                if (tex) {
                    sf::Sprite groundSprite(*tex);

                    // If the definition specifies a cut of the texture, use it
                    groundSprite.setPosition({
                        static_cast<float>(x * GameConfig::TILE_SIZE),
                        static_cast<float>(y * GameConfig::TILE_SIZE)
                    });
                    window.draw(groundSprite);
                }
            }
        }

        // --- 2. COLLECT EVERYTHING ELSE (Layer 1) ---
        std::vector<RenderPacket> renderQueue;

        // A. Add Trees/Rocks (World Objects)
        for (const auto& obj : worldMap.getStructures()) {
            RenderPacket packet;
            packet.sortY = obj->getSortY(); // Uses position.y + height

            // Capture the object by reference and draw it
            packet.drawFunc = [&](sf::RenderWindow& w) {
                // Casting away const just to call draw (if your draw isn't const)
                const_cast<WorldObject&>(*obj).draw(w);
                };
            renderQueue.push_back(packet);
        }

        // B. Add NPCs
        for (const auto& npc : worldMap.getNPCs()) {
            RenderPacket packet;
            packet.sortY = npc->getPosition().y + npc->getGlobalBounds().size.y; // Feet Y
            packet.drawFunc = [&](sf::RenderWindow& w) { npc->draw(w); };
            renderQueue.push_back(packet);
        }

        // C. Add Player
        RenderPacket playerPacket;
        playerPacket.sortY = player.getPosition().y + player.getGlobalBounds().size.y; // Feet Y
        playerPacket.drawFunc = [&](sf::RenderWindow& w) {
            const_cast<Player&>(player).draw(w);
            };
        renderQueue.push_back(playerPacket);

        // --- 3. SORT ---
        // This is the magic. Sorts everything by their 'sortY' value.
        std::sort(renderQueue.begin(), renderQueue.end());

        // --- 4. RENDER ---
        for (const auto& packet : renderQueue) {
            packet.drawFunc(window);
        }
    }
}