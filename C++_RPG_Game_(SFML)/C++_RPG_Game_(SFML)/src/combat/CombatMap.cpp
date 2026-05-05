#include "CombatMap.h"
#include "combat/Unit.h"
#include "core/IsoHelpers.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>

namespace RPG {

    void CombatMap::addObject(std::shared_ptr<CombatWorldObject> obj) {
        if (obj) objects.push_back(obj);
    }

    void CombatMap::removeObject(std::shared_ptr<CombatWorldObject> obj) {
        objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end());
    }

    const std::vector<std::shared_ptr<CombatWorldObject>>& CombatMap::getAllObjects() const { return objects; }

    bool CombatMap::isBlocked(const sf::Vector2f& point, CombatWorldObject* ignoreObject) const {
        for (const auto& obj : objects) {
            if (obj.get() == ignoreObject) continue;
            if (!obj->blocksMovement()) continue;
            if (obj->getCollider().contains(point)) return true;
        }
        return false;
    }

    std::shared_ptr<CombatWorldObject> CombatMap::getHitObject(const sf::Vector2f& point) const {
        for (const auto& obj : objects) {
            if (obj->getCollider().contains(point)) return obj;
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<Unit>> CombatMap::getUnitsInRadius(const sf::Vector2f& center, float radius) const {
        std::vector<std::shared_ptr<Unit>> caughtUnits;
        for (const auto& obj : objects) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(obj)) {
                float dist = Iso::getDistance(center, unit->getLogicalPosition());
                if (dist <= radius) {
                    caughtUnits.push_back(unit);
                }
            }
        }
        return caughtUnits;
    }

    // A* Helper Structs
    struct Node {
        sf::Vector2i gridPos;
        float gCost = 0.f;
        float hCost = 0.f;
        float fCost() const { return gCost + hCost; }
        sf::Vector2i parent;

        bool operator>(const Node& other) const {
            if (fCost() == other.fCost()) return hCost > other.hCost;
            return fCost() > other.fCost();
        }
    };

    struct Vector2iHash {
        std::size_t operator()(const sf::Vector2i& v) const {
            return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
        }
    };

    std::vector<sf::Vector2f> CombatMap::findPath(const sf::Vector2f& start, const sf::Vector2f& end, const CombatWorldObject* ignoreObj) const {
        std::vector<sf::Vector2f> path;

        sf::Vector2i startGrid(static_cast<int>(start.x / m_tileSize), static_cast<int>(start.y / m_tileSize));
        sf::Vector2i endGrid(static_cast<int>(end.x / m_tileSize), static_cast<int>(end.y / m_tileSize));

        if (endGrid.x < 0 || endGrid.x >= m_gridWidth || endGrid.y < 0 || endGrid.y >= m_gridHeight) return path;
        if (isBlocked(end, const_cast<CombatWorldObject*>(ignoreObj))) return path;

        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openList;
        std::unordered_map<sf::Vector2i, Node, Vector2iHash> allNodes;
        std::unordered_map<sf::Vector2i, bool, Vector2iHash> closedList;

        Node startNode{ startGrid, 0.f, Iso::getDistance(start, end) };
        startNode.parent = startGrid;
        openList.push(startNode);
        allNodes[startGrid] = startNode;

        std::vector<sf::Vector2i> directions = {
            {0, -1}, {0, 1}, {-1, 0}, {1, 0},
            {-1, -1}, {1, -1}, {-1, 1}, {1, 1}
        };

        bool foundPath = false;

        while (!openList.empty()) {
            Node current = openList.top();
            openList.pop();

            if (current.gridPos == endGrid) {
                foundPath = true;
                break;
            }

            if (closedList[current.gridPos]) continue;
            closedList[current.gridPos] = true;

            for (const auto& dir : directions) {
                sf::Vector2i neighborPos = current.gridPos + dir;

                if (neighborPos.x < 0 || neighborPos.x >= m_gridWidth || neighborPos.y < 0 || neighborPos.y >= m_gridHeight) continue;
                if (closedList[neighborPos]) continue;

                sf::Vector2f worldCheck(neighborPos.x * m_tileSize + m_tileSize / 2.f, neighborPos.y * m_tileSize + m_tileSize / 2.f);
                if (isBlocked(worldCheck, const_cast<CombatWorldObject*>(ignoreObj))) continue;

                float moveCost = (dir.x != 0 && dir.y != 0) ? 1.414f : 1.0f;
                float newGCost = current.gCost + moveCost;

                if (allNodes.find(neighborPos) == allNodes.end() || newGCost < allNodes[neighborPos].gCost) {
                    Node neighborNode;
                    neighborNode.gridPos = neighborPos;
                    neighborNode.gCost = newGCost;
                    neighborNode.hCost = Iso::getDistance(worldCheck, end) / m_tileSize;
                    neighborNode.parent = current.gridPos;

                    allNodes[neighborPos] = neighborNode;
                    openList.push(neighborNode);
                }
            }
        }

        if (foundPath) {
            sf::Vector2i curr = endGrid;

            // Trace the path backward from end to start
            while (curr != startGrid) {
                path.push_back({ curr.x * m_tileSize + m_tileSize / 2.f, curr.y * m_tileSize + m_tileSize / 2.f });
                curr = allNodes[curr].parent;
            }

            // Check if the path is actually populated before modifying the front!
            if (!path.empty()) {
                path.front() = end; // Smooth out the final step to exact mouse click
                std::reverse(path.begin(), path.end());
            }
            else {
                // Edge Case: startGrid == endGrid (moving within the exact same 32x32 tile)
                // Just add the exact clicked point as the only waypoint.
                path.push_back(end);
            }
        }

        return path;
    }

    bool CombatMap::hasLineOfSight(const sf::Vector2f& start, const sf::Vector2f& end) const {
        float dist = Iso::getDistance(start, end);
        if (dist < 0.1f) return true;
        sf::Vector2f dir = (end - start) / dist;
        const float stepSize = 5.0f;
        int steps = static_cast<int>(dist / stepSize);
        sf::Vector2f current = start;

        for (int i = 0; i < steps; ++i) {
            current += dir * stepSize;
            for (const auto& obj : objects) {
                if (obj->blocksSight() && obj->getCollider().contains(current)) {
                    return false;
                }
            }
        }
        return true;
    }

    bool CombatMap::canMoveDirectly(const sf::Vector2f& start, const sf::Vector2f& end, const CombatWorldObject* ignoreObj) const {
        float dist = Iso::getDistance(start, end);
        if (dist < 1.0f) return true;

        sf::Vector2f dir = (end - start) / dist;
        const float stepSize = 10.0f;
        int steps = static_cast<int>(dist / stepSize);
        sf::Vector2f current = start;

        for (int i = 1; i <= steps; ++i) {
            current += dir * stepSize;
            for (const auto& obj : objects) {
                if (obj.get() == ignoreObj) continue;
                if (obj->blocksMovement() && obj->getCollider().contains(current)) {
                    return false;
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
            sf::Vector2u texSize = m_backgroundSprite->getTexture().getSize();
            float scaleX = (float)newSize.x / texSize.x;
            float scaleY = (float)newSize.y / texSize.y;
            m_backgroundSprite->setScale({ scaleX, scaleY });
        }
    }

    void CombatMap::loadFromTiles(const sf::Texture& tileset, sf::Vector2u tileSize, int mapWidth, int mapHeight, const std::vector<int>& tileData) {
        m_mode = MapMode::Tiled;
        m_tileset = &tileset;
        m_gridWidth = mapWidth;
        m_gridHeight = mapHeight;
        m_tileSize = (float)tileSize.x;

        m_vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
        m_vertices.resize(mapWidth * mapHeight * 6);

        for (int i = 0; i < mapWidth; ++i) {
            for (int j = 0; j < mapHeight; ++j) {

                int tileNumber = tileData[i + j * mapWidth];
                if (tileNumber < 0) continue;

                int tu = tileNumber % (tileset.getSize().x / tileSize.x);
                int tv = tileNumber / (tileset.getSize().x / tileSize.x);

                sf::Vector2f texPos(tu * tileSize.x, tv * tileSize.y);
                sf::Vector2f texSize(tileSize.x, tileSize.y);

                float logicSize = m_tileSize;

                sf::Vector2f p1 = Iso::worldToScreen({ i * logicSize, j * logicSize });
                sf::Vector2f p2 = Iso::worldToScreen({ (i + 1) * logicSize, j * logicSize });
                sf::Vector2f p3 = Iso::worldToScreen({ (i + 1) * logicSize, (j + 1) * logicSize });
                sf::Vector2f p4 = Iso::worldToScreen({ i * logicSize, (j + 1) * logicSize });

                int idx = (i + j * mapWidth) * 6;

                m_vertices[idx + 0].position = p1; m_vertices[idx + 0].texCoords = texPos;
                m_vertices[idx + 1].position = p2; m_vertices[idx + 1].texCoords = texPos + sf::Vector2f(texSize.x, 0.f);
                m_vertices[idx + 2].position = p4; m_vertices[idx + 2].texCoords = texPos + sf::Vector2f(0.f, texSize.y);

                m_vertices[idx + 3].position = p2; m_vertices[idx + 3].texCoords = texPos + sf::Vector2f(texSize.x, 0.f);
                m_vertices[idx + 4].position = p3; m_vertices[idx + 4].texCoords = texPos + texSize;
                m_vertices[idx + 5].position = p4; m_vertices[idx + 5].texCoords = texPos + sf::Vector2f(0.f, texSize.y);
            }
        }
    }

    void CombatMap::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        if (m_backgroundSprite.has_value()) {
            sf::View currentView = target.getView();
            sf::Vector2f targetSize((float)target.getSize().x, (float)target.getSize().y);
            sf::View bgView(sf::FloatRect({ 0.f, 0.f }, targetSize));

            target.setView(bgView);
            target.draw(*m_backgroundSprite);
            target.setView(currentView);
        }

        if (m_mode == MapMode::SingleImage && m_singleFloorSprite.has_value()) {
            target.draw(*m_singleFloorSprite, states);
        }
        else if (m_mode == MapMode::Tiled && m_tileset) {
            states.texture = m_tileset;
            target.draw(m_vertices, states);
        }
    }
}