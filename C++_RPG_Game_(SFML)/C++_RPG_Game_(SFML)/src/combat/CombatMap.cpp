#include "CombatMap.h"
#include "core/IsoHelpers.h"
#include <algorithm>
#include <cmath>

namespace RPG {

    void CombatMap::addObject(std::shared_ptr<CombatWorldObject> obj) {
        if (obj) {
            objects.push_back(obj);
        }
    }

    void CombatMap::removeObject(std::shared_ptr<CombatWorldObject> obj) {
        objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end());
    }

    const std::vector<std::shared_ptr<CombatWorldObject>>& CombatMap::getAllObjects() const {
        return objects;
    }

    bool CombatMap::isBlocked(const sf::Vector2f& point, CombatWorldObject* ignoreObject) const {
        for (const auto& obj : objects) {
            // Skip the object if it's the one currently moving
            if (obj.get() == ignoreObject) continue;

            // Skip non-blocking objects (like grass or rugs)
            if (!obj->blocksMovement()) continue;

            // SFML 3: Rect::contains takes a Vector2
            if (obj->getCollider().contains(point)) {
                return true;
            }
        }
        return false;
    }

    std::shared_ptr<CombatWorldObject> CombatMap::getHitObject(const sf::Vector2f& point) const {
        for (const auto& obj : objects) {
            // SFML 3: Rect::contains takes a Vector2
            if (obj->getCollider().contains(point)) {
                return obj;
            }
        }
        return nullptr;
    }

    bool CombatMap::hasLineOfSight(const sf::Vector2f& start, const sf::Vector2f& end) const {
        float dist = Iso::getDistance(start, end);

        // If start and end are practically the same, we have sight.
        if (dist < 0.1f) return true;

        sf::Vector2f dir = (end - start) / dist;

        // Step size for raycasting
        // A smaller step is more accurate but computationally heavier.
        // 5.0f is a reasonable balance for standard unit scales
        const float stepSize = 5.0f;
        int steps = static_cast<int>(dist / stepSize);

        sf::Vector2f current = start;

        for (int i = 0; i < steps; ++i) {
            current += dir * stepSize;

            for (const auto& obj : objects) {
                // We only care about objects that specifically block sight (Walls vs Fences)
                if (obj->blocksSight()) {
                    // SFML 3: Rect::contains takes a Vector2
                    if (obj->getCollider().contains(current)) {
                        return false; // Vision Blocked
                    }
                }
            }
        }

        return true; // Line of Sight Clear
    }

    bool CombatMap::canMoveDirectly(const sf::Vector2f& start, const sf::Vector2f& end, const CombatWorldObject* ignoreObj) const {
        float dist = Iso::getDistance(start, end);

        // If distance is tiny, technically it's clear, but isValidMove will reject it later
        if (dist < 1.0f) return true;

        sf::Vector2f dir = (end - start) / dist;
        const float stepSize = 10.0f;
        int steps = static_cast<int>(dist / stepSize);

        sf::Vector2f current = start;

        for (int i = 1; i <= steps; ++i) {
            current += dir * stepSize;

            for (const auto& obj : objects) {
                if (obj.get() == ignoreObj) continue;

                if (obj->blocksMovement()) {
                    if (obj->getCollider().contains(current)) {
                        return false; // Path Blocked
                    }
                }
            }
        }
        return true;
    }

    void CombatMap::loadFromImage(const sf::Texture& image) {
        m_mode = MapMode::SingleImage;
        m_singleFloorSprite.emplace(image);
        m_singleFloorSprite->setOrigin({ image.getSize().x / 2.f, 0.f });
        m_singleFloorSprite->setPosition(Iso::worldToScreen({ 0.f, 0.f }));
    }

    void CombatMap::setBackground(const sf::Texture& bgTexture) {
        m_backgroundSprite.emplace(bgTexture);
    }
    void CombatMap::onResize(sf::Vector2u newSize) {
        if (m_backgroundSprite.has_value()) {
            // Calculate Scale Factor
            sf::Vector2u texSize = m_backgroundSprite->getTexture().getSize();

            float scaleX = (float)newSize.x / texSize.x;
            float scaleY = (float)newSize.y / texSize.y;

            m_backgroundSprite->setScale({ scaleX, scaleY });
        }
    }
    void CombatMap::loadFromTiles(const sf::Texture& tileset, sf::Vector2u tileSize, int mapWidth, int mapHeight, const std::vector<int>& tileData) {
        m_mode = MapMode::Tiled;
        m_tileset = &tileset;

        // Resize vertex array to hold (width * height * 4 vertices)
        m_vertices.setPrimitiveType(sf::PrimitiveType::Triangles); // SFML 3 prefers Triangles over Quads usually, but let's use TriangleStrip or Triangles
        m_vertices.resize(mapWidth * mapHeight * 6); // 6 vertices per tile (2 triangles)

        for (int i = 0; i < mapWidth; ++i) {
            for (int j = 0; j < mapHeight; ++j) {

                // 1. Get Tile Index
                int tileNumber = tileData[i + j * mapWidth]; // Simple 1D array access
                if (tileNumber < 0) continue; // Empty tile

                // 2. Calculate Texture Coordinates
                // Assuming tileset is a grid
                int tu = tileNumber % (tileset.getSize().x / tileSize.x);
                int tv = tileNumber / (tileset.getSize().x / tileSize.x);

                sf::Vector2f texPos(tu * tileSize.x, tv * tileSize.y);
                sf::Vector2f texSize(tileSize.x, tileSize.y);

                // 3. Calculate World Position (Corners of the tile)
                // Logical Grid Size is usually GameConfig::TILE_SIZE (32.f)
                float logicSize = 32.f;

                sf::Vector2f p1 = Iso::worldToScreen({ i * logicSize, j * logicSize });          // Top
                sf::Vector2f p2 = Iso::worldToScreen({ (i + 1) * logicSize, j * logicSize });      // Right
                sf::Vector2f p3 = Iso::worldToScreen({ (i + 1) * logicSize, (j + 1) * logicSize });  // Bottom
                sf::Vector2f p4 = Iso::worldToScreen({ i * logicSize, (j + 1) * logicSize });      // Left

                // 4. Map to Vertices (2 Triangles)
                // Triangle 1: p1, p2, p4
                int idx = (i + j * mapWidth) * 6;

                m_vertices[idx + 0].position = p1; m_vertices[idx + 0].texCoords = texPos;
                m_vertices[idx + 1].position = p2; m_vertices[idx + 1].texCoords = texPos + sf::Vector2f(texSize.x, 0.f);
                m_vertices[idx + 2].position = p4; m_vertices[idx + 2].texCoords = texPos + sf::Vector2f(0.f, texSize.y);

                // Triangle 2: p2, p3, p4
                m_vertices[idx + 3].position = p2; m_vertices[idx + 3].texCoords = texPos + sf::Vector2f(texSize.x, 0.f);
                m_vertices[idx + 4].position = p3; m_vertices[idx + 4].texCoords = texPos + texSize;
                m_vertices[idx + 5].position = p4; m_vertices[idx + 5].texCoords = texPos + sf::Vector2f(0.f, texSize.y);
            }
        }
    }

    void CombatMap::draw(sf::RenderTarget& target, sf::RenderStates states) const {

        // 1. Draw Static Background
        // Reset view for background so it stays static on screen
        /*if (m_backgroundSprite.has_value()) {
            sf::View currentView = target.getView();

            sf::Vector2f targetSize((float)target.getSize().x, (float)target.getSize().y);
            sf::View bgView(sf::FloatRect({ 0.f, 0.f }, targetSize));

            target.setView(bgView);
            target.draw(*m_backgroundSprite);

            target.setView(currentView);
        }*/

        // 2. Draw Floor
        if (m_mode == MapMode::SingleImage && m_singleFloorSprite.has_value()) {
            target.draw(*m_singleFloorSprite, states);
        }
        else if (m_mode == MapMode::Tiled && m_tileset) {
            states.texture = m_tileset;
            target.draw(m_vertices, states);
        }
    }

}