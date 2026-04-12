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
        float percentView
    ) {
        // --- SETUP VIEW ---
        sf::View currentView = window.getView();
        sf::Vector2f center = currentView.getCenter();
        sf::Vector2f size = currentView.getSize();

        float zoomFactor = 2.0f;

        // 2. Get the actual window size (e.g., 1280x720)
        sf::Vector2u windowSize = window.getSize();

        // 3. Create a view that is smaller than the window
        sf::View view;
        view.setSize(
            { windowSize.x / zoomFactor,
            windowSize.y / zoomFactor }
        );

        // 4. Center it on the player (World Coordinates)
        view.setCenter(player.getPosition());

        // 5. Apply it
        window.setView(view);

        // Calculate visible range (plus margin to prevent popping)
        int32_t startX = std::max<int32_t>(0, (center.x - size.x / 2) / GameConfig::TILE_SIZE - 2);
        int32_t startY = std::max<int32_t>(0, (center.y - size.y / 2) / GameConfig::TILE_SIZE - 2);
        int32_t endX = std::min<int32_t>(worldMap.getWidth(), (center.x + size.x / 2) / GameConfig::TILE_SIZE + 3);
        int32_t endY = std::min<int32_t>(worldMap.getHeight(), (center.y + size.y / 2) / GameConfig::TILE_SIZE + 3);

        // ==============================
        // 1. DRAW GROUND (Layer 0)
        // ==============================

        // Cache texture pointer to avoid lookups in the loop
        const sf::Texture* grassTex = assetManager.getTexture("grass");

        if (grassTex) {
            sf::Sprite groundSprite(*grassTex);

            for (int32_t y = startY; y < endY; ++y) {
                for (int32_t x = startX; x < endX; ++x) {
                    groundSprite.setPosition({
                        static_cast<float>(x * GameConfig::TILE_SIZE),
                        static_cast<float>(y * GameConfig::TILE_SIZE)
                    });
                    window.draw(groundSprite);
                }
            }
        }

        // ==============================
        // 2. DRAW OBJECTS & ENTITIES (Layer 1)
        // ==============================
        std::vector<RenderPacket> renderQueue;

        // A. World Objects
        for (const auto& obj : worldMap.getStructures()) {
            RenderPacket packet;
            packet.sortY = obj->getSortY();
            packet.drawFunc = [&](sf::RenderWindow& w) {
                const_cast<WorldObject&>(*obj).draw(w);
                };
            renderQueue.push_back(packet);
        }

        // B. NPCs
        for (const auto& npc : worldMap.getNPCs()) {
            RenderPacket packet;
            packet.sortY = npc->getPosition().y + npc->getGlobalBounds().size.y * 0.5; // change -> ADD HITBOX
            packet.drawFunc = [&](sf::RenderWindow& w) { npc->draw(w); };
            renderQueue.push_back(packet);
        }

        // C. Player
        RenderPacket playerPacket;
        playerPacket.sortY = player.getPosition().y + player.getHitbox().position.y + player.getHitbox().size.y;
        playerPacket.drawFunc = [&](sf::RenderWindow& w) {
            const_cast<Player&>(player).draw(w);
            };
        renderQueue.push_back(playerPacket);

        // Sort & Draw
        std::sort(renderQueue.begin(), renderQueue.end());
        for (const auto& packet : renderQueue) {
            packet.drawFunc(window);
        }



        // NEW! RENDERING GAMEOBJECTS


        std::vector<GameObject*> queue;

        // TODO: ADD PLAYER

        for (const auto& go : worldMap.getGameObjects()) {
            queue.push_back(go.get());
        }

        std::sort(queue.begin(), queue.end(), [](GameObject* a, GameObject* b) {
            return a->getPosition().y < b->getPosition().y;
        });

        for (auto& obj : queue) {
            obj->draw(window);
        }

        // ==============================
        // 3. DRAW FOG OF WAR (Layer 2)
        // ==============================

        // 1. Calculate Radius based on 'percentView'
        // We take the smaller screen dimension (width or height) to ensure the circle fits.
        // If percentView is 1.0, the radius reaches the edge of the screen.
        float minScreenDim = std::min(size.x, size.y);
        float visibleRadius = (minScreenDim / 2.0) * percentView;

        // Optional: Scale fade width nicely with the radius (e.g., 20% of the clear zone)
        // or keep it static. Here we make it dynamic for smoother zooming.
        float fadeWidth = visibleRadius * 0.4;
        sf::Vector2f playerPos = player.getPosition();

        // ZMIANA 1: U¿ywamy Triangles zamiast Quads
        // W SFML 3.0 typy s¹ w enum class PrimitiveType
        sf::VertexArray fogLayer(sf::PrimitiveType::Triangles);

        for (int32_t y = startY; y < endY; ++y) {
            for (int32_t x = startX; x < endX; ++x) {
                float tileWorldX = x * GameConfig::TILE_SIZE + (GameConfig::TILE_SIZE / 2.f);
                float tileWorldY = y * GameConfig::TILE_SIZE + (GameConfig::TILE_SIZE / 2.f);

                float dx = tileWorldX - playerPos.x;
                float dy = tileWorldY - playerPos.y;
                float dist = std::sqrt(dx * dx + dy * dy);

                uint8_t alpha = 255;

                if (dist < visibleRadius) {
                    alpha = 0;
                }
                else if (dist < visibleRadius + fadeWidth) {
                    float factor = (dist - visibleRadius) / fadeWidth;
                    alpha = static_cast<uint8_t>(factor * 255);
                }

                if (alpha == 0) continue;

                // Wspó³rzêdne kafelka
                float tx = static_cast<float>(x * GameConfig::TILE_SIZE);
                float ty = static_cast<float>(y * GameConfig::TILE_SIZE);
                float ts = static_cast<float>(GameConfig::TILE_SIZE);

                sf::Color fogColor(96, 96, 48, alpha);

                // ZMIANA 2: Musimy zdefiniowaæ 6 wierzcho³ków (2 trójk¹ty) zamiast 4

                // Trójk¹t 1 (Lewy-Górny, Prawy-Górny, Lewy-Dolny)
                fogLayer.append(sf::Vertex({ tx, ty }, fogColor));          // Top-Left
                fogLayer.append(sf::Vertex({ tx + ts, ty }, fogColor));     // Top-Right
                fogLayer.append(sf::Vertex({ tx, ty + ts }, fogColor));     // Bottom-Left

                // Trójk¹t 2 (Prawy-Górny, Prawy-Dolny, Lewy-Dolny)
                fogLayer.append(sf::Vertex({ tx + ts, ty }, fogColor));     // Top-Right
                fogLayer.append(sf::Vertex({ tx + ts, ty + ts }, fogColor));// Bottom-Right
                fogLayer.append(sf::Vertex({ tx, ty + ts }, fogColor));     // Bottom-Left
            }
        }

        window.draw(fogLayer);
    }
}