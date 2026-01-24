#pragma once
#include <SFML/System/Vector2.hpp>
#include <cmath>

namespace RPG {

    /**
     * @namespace Iso
     * @brief Collection of static helper functions for Isometric coordinate conversions.
     * * This namespace handles the mathematical projection between the Logical 2D Grid (Top-Down)
     * and the Visual Isometric Screen (2:1 projection).
     * * Coordinate Systems:
     * - **World Space (Logical):** Cartesian grid. +X is Right, +Y is Down.
     * Used for physics, distance checks, and game logic.
     * - **Screen Space (Visual):** Isometric projection.
     * Used for sprite positioning and drawing.
     */
    namespace Iso {

        /**
         * @brief The scaling factor for the isometric projection.
         * * Determines the visual width of a single logical unit.
         * Even in a "tile-less" system, this scales the world to the screen.
         * Defaulting to 1.0f means 1 logical unit = 1 pixel.
         * Adjust this to match sprite scale.
         */
        constexpr float ISO_SCALE = 1.0f;

        /**
         * @brief Converts a Logical World position to an Isometric Screen position.
         * * Used during the Rendering phase.
         * Formula:
         * ScreenX = (WorldX - WorldY) * Scale
         * ScreenY = (WorldX + WorldY) * 0.5 * Scale
         * * @param worldPos The logical (x, y) coordinates in the game world.
         * @return sf::Vector2f The screen pixel coordinates to draw the sprite.
         */
        inline sf::Vector2f worldToScreen(const sf::Vector2f& worldPos) {
            sf::Vector2f screenPos;
            screenPos.x = (worldPos.x - worldPos.y) * ISO_SCALE;
            screenPos.y = (worldPos.x + worldPos.y) * 0.5f * ISO_SCALE;
            return screenPos;
        }

        /**
         * @brief Converts an Isometric Screen position back to Logical World coordinates.
         * * Used during the Input phase (Mouse clicks).
         * Formula:
         * WorldX = (ScreenY / 0.5 + ScreenX) / 2
         * WorldY = (ScreenY / 0.5 - ScreenX) / 2
         * * @param screenPos The pixel coordinates on the screen (e.g., mouse position).
         * @return sf::Vector2f The logical (x, y) coordinates in the game world.
         */
        inline sf::Vector2f screenToWorld(const sf::Vector2f& screenPos) {
            sf::Vector2f worldPos;

            // Reverse scaling first
            float scaledX = screenPos.x / ISO_SCALE;
            float scaledY = screenPos.y / (0.5f * ISO_SCALE);

            worldPos.x = (scaledY + scaledX) * 0.5f;
            worldPos.y = (scaledY - scaledX) * 0.5f;

            return worldPos;
        }

        /**
         * @brief Calculates the Render Depth (Z-Order) for a given position.
         * * In isometric projection, objects "lower" on the screen (higher Y)
         * or "further right" (higher X) should be drawn last to appear in front.
         * * @param worldPos The logical world position of the object's base/feet.
         * @return float A depth value used for sorting render order.
         */
        inline float getRenderDepth(const sf::Vector2f& worldPos) {
            // Simple sum of X and Y correlates directly to the screen Y line.
            return (worldPos.x + worldPos.y);
        }

        /**
         * @brief Calculates the Euclidean distance between two logical points.
         * * Since logic happens in standard Cartesian space, we use standard Pythagoras.
         * * @param a The first point.
         * @param b The second point.
         * @return float The distance between a and b.
         */
        inline float getDistance(const sf::Vector2f& a, const sf::Vector2f& b) {
            float dx = a.x - b.x;
            float dy = a.y - b.y;
            return std::sqrt(dx * dx + dy * dy);
        }
    }
}