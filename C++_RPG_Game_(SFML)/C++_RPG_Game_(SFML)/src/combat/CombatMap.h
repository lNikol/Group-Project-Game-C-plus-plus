#pragma once
#include "CombatWorldObject.h"
#include <vector>
#include <memory>
#include <optional>
#include "Unit.h"
namespace RPG {
    enum class MapMode {
        None,
        Tiled,
        SingleImage
    };

    /**
     * @brief The container for the battle arena.
     * * Holds all Units and Props, responsible for spatial queries, A* Pathfinding,
     * and Area of Effect radius scanning.
     */
    class CombatMap : public sf::Drawable {
    public:
        CombatMap() = default;
        ~CombatMap() = default;

        void addObject(std::shared_ptr<CombatWorldObject> obj);
        void removeObject(std::shared_ptr<CombatWorldObject> obj);
        const std::vector<std::shared_ptr<CombatWorldObject>>& getAllObjects() const;

        // ==============================
        // Spatial Queries
        // ==============================
        bool isBlocked(const sf::Vector2f& point, CombatWorldObject* ignoreObject = nullptr) const;
        std::shared_ptr<CombatWorldObject> getHitObject(const sf::Vector2f& point) const;
        bool hasLineOfSight(const sf::Vector2f& start, const sf::Vector2f& end) const;
        bool canMoveDirectly(const sf::Vector2f& start, const sf::Vector2f& end, const CombatWorldObject* ignoreObj) const;

        /**
         * @brief Finds all units within a specific logical radius of a center point.
         * @param center The logical (x,y) impact point of the spell.
         * @param radius The logical radius of the spell.
         * @return A list of valid Unit pointers caught in the area.
         */
        std::vector<std::shared_ptr<Unit>> getUnitsInRadius(const sf::Vector2f& center, float radius) const;

        /**
         * @brief Finds the shortest path between two points using the A* algorithm.
         * @param start The logical starting coordinate.
         * @param end The logical destination coordinate.
         * @param ignoreObj The unit that is moving.
         * @return A list of logical coordinates (waypoints) forming the path. Empty if blocked.
         */
        std::vector<sf::Vector2f> findPath(const sf::Vector2f& start, const sf::Vector2f& end, const CombatWorldObject* ignoreObj = nullptr) const;

        // ==============================
        // Terrain / Visuals
        // ==============================
        void loadFromTiles(const sf::Texture& tileset, sf::Vector2u tileSize, int mapWidth, int mapHeight, const std::vector<int>& tileData);
        void loadFromImage(const sf::Texture& image);
        void setBackground(const sf::Texture& bgTexture);
        void onResize(sf::Vector2u newSize);

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        int m_gridWidth = 15;
        int m_gridHeight = 15;
        float m_tileSize = 32.f;

        std::vector<std::shared_ptr<CombatWorldObject>> objects;
        MapMode m_mode = MapMode::None;
        std::optional<sf::Sprite> m_singleFloorSprite;
        std::optional<sf::Sprite> m_backgroundSprite;
        sf::VertexArray m_vertices;
        const sf::Texture* m_tileset = nullptr;
    };
}