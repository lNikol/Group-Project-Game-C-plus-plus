#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

namespace RPG {

    /**
     * @brief Renders a staggered isometric grid using a single vertex array for performance.
     * * This class generates a mesh of "pseudo-3D" cubes. It inherits from sf::Transformable,
     * allowing the entire grid to be moved, scaled, or rotated as a single unit.
     * The grid uses simulated lighting by calculating different shades for the Top,
     * Left, and Right faces of every tile.
     */
    class IsoGrid : public sf::Drawable, public sf::Transformable {
    public:
        /**
         * @brief Constructs the isometric grid and builds the initial mesh.
         * * @param rows The number of rows (vertical height of the grid).
         * @param cols The number of columns (horizontal width of the grid).
         * @param tileSize The size (radius) of each individual tile/cube.
         * @param baseColor The fundamental color of the grid. Lighting calculations are applied relative to this.
         */
        IsoGrid(int rows, int cols, float tileSize, sf::Color baseColor);

        /**
         * @brief Clears the current mesh and generates a new one.
         * * This calculates the staggered positions for the isometric layout and
         * applies alternating color offsets for visual distinction between tiles.
         * * @param rows The number of rows.
         * @param cols The number of columns.
         * @param tileSize The size of the tiles.
         * @param baseColor The base color to regenerate the grid with.
         */
        void rebuild(int rows, int cols, float tileSize, sf::Color baseColor);

    private:
        /**
         * @brief Container for all vertex data (using PrimitiveType::Triangles).
         */
        sf::VertexArray m_vertices;

        /**
         * @brief Helper function to generate the geometry for a single isometric cube.
         * * It generates 3 faces (Top, Left, Right) and applies simulated lighting:
         * - Top: Brighter than baseColor.
         * - Left: Slightly darker (medium shadow).
         * - Right: Darkest (deep shadow).
         * * @param x The screen-space X coordinate of the cube center.
         * @param y The screen-space Y coordinate of the cube center.
         * @param size The geometric size/radius of the cube.
         * @param color The specific base color for this tile (before lighting).
         */
        void addIsoCube(float x, float y, float size, sf::Color color);

        /**
         * @brief Helper to append two triangles (making one quad) to the vertex array.
         * * @param p1 First vertex position.
         * @param p2 Second vertex position.
         * @param p3 Third vertex position.
         * @param p4 Fourth vertex position.
         * @param color The color to apply to all vertices in this quad.
         */
        void addQuad(sf::Vector2f p1, sf::Vector2f p2, sf::Vector2f p3, sf::Vector2f p4, sf::Color color);

        /**
         * @brief SFML rendering entry point.
         * * Applies the object's transform and draws the vertex array to the target.
         * * @param target The render target (window or texture).
         * @param states The current render states.
         */
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    };
}