#pragma once
#include "CombatWorldObject.h"
#include <vector>
#include <memory>
#include <optional>

namespace RPG {
    enum class MapMode {
        None,
        Tiled,
        SingleImage
    };
    /**
     * @brief The container for the battle arena.
     * * This class holds all Units and Props. It is responsible for spatial queries
     * (collisions, clicking, line of sight).
     */
    class CombatMap : public sf::Drawable{
    public:
        CombatMap() = default;
        ~CombatMap() = default;

        /**
         * @brief Adds an object to the map.
         * @param obj A shared pointer to the object (Prop or Unit).
         */
        void addObject(std::shared_ptr<CombatWorldObject> obj);

        /**
         * @brief Removes an object from the map (e.g., destroyed prop, dead body).
         * @param obj The object to remove.
         */
        void removeObject(std::shared_ptr<CombatWorldObject> obj);

        /**
         * @brief Gets the list of all objects, primarily for the Renderer.
         * * Note: The Renderer will need to sort this list by getRenderDepth() every frame.
         * @return const std::vector<std::shared_ptr<WorldObject>>&
         */
        const std::vector<std::shared_ptr<CombatWorldObject>>& getAllObjects() const;

        // ==============================
        // Spatial Queries
        // ==============================

        /**
         * @brief Checks if a specific point in the world is blocked.
         * * Useful for checking if a unit can walk to a target point.
         * @param point The logical (x,y) coordinate.
         * @param ignoreObject Optional object to ignore (e.g., the unit itself moving).
         * @return true If the point is inside a blocking object.
         */
        bool isBlocked(const sf::Vector2f& point, CombatWorldObject* ignoreObject = nullptr) const;

        /**
         * @brief Finds the object at a specific coordinate.
         * * This is the core method for Mouse Selection.
         * @param point The logical (x,y) coordinate (converted from mouse click).
         * @return std::shared_ptr<WorldObject> The object found, or nullptr if empty ground.
         */
        std::shared_ptr<CombatWorldObject> getHitObject(const sf::Vector2f& point) const;

        /**
         * @brief Basic Line of Sight check.
         * * Casts a ray from start to end to see if it hits any object with blocksSight() == true.
         * @param start Origin point.
         * @param end Destination point.
         * @return true If the line is clear.
         */
        bool hasLineOfSight(const sf::Vector2f& start, const sf::Vector2f& end) const;
        /**
          * @brief Checks if a straight line from start to end is clear of obstacles.
          * @param start World coordinate of start point.
          * @param end World coordinate of target point.
          * @param ignoreObj Pointer to the object moving (so it doesn't block itself).
          */
        bool canMoveDirectly(const sf::Vector2f& start, const sf::Vector2f& end, const CombatWorldObject* ignoreObj) const;
        // ==============================
        // Terrain / Visuals
        // ==============================

        /**
        * @brief Option A: Load a map from a grid of tiles.
        * @param tileset The texture containing the tile images.
        * @param tileSize The size of one tile in the texture (e.g., 32x32).
        * @param mapWidth Width in tiles.
        * @param mapHeight Height in tiles.
        * @param tileData Vector of indices (pointing to which tile to draw from the set).
        */
        void loadFromTiles(const sf::Texture& tileset, sf::Vector2u tileSize, int mapWidth, int mapHeight, const std::vector<int>& tileData);

        /**
         * @brief Option B: Load a map from a single pre-rendered image.
         * @param image The texture of the full floor.
         */
        void loadFromImage(const sf::Texture& image);

        /**
         * @brief Sets a static background image (sky/void).
         */
        void setBackground(const sf::Texture& bgTexture);
        /**
         * @brief Updates background scaling to cover the new window size.
         */
        void onResize(sf::Vector2u newSize);

    protected:
        // Override draw to render the Floor and Background
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        // Objects
        std::vector<std::shared_ptr<CombatWorldObject>> objects;

        // Visuals
        MapMode m_mode = MapMode::None;

        // Single Image Data
        std::optional<sf::Sprite> m_singleFloorSprite;
        std::optional<sf::Sprite> m_backgroundSprite;

        // Tiled Data
        sf::VertexArray m_vertices;
        const sf::Texture* m_tileset = nullptr;
    };
}