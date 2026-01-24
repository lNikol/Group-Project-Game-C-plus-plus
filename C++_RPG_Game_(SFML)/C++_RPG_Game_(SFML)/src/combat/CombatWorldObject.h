#pragma once
#include <SFML/Graphics.hpp>
#include "core/IsoHelpers.h"
#include <memory>

namespace RPG {

    /**
     * @brief Base class for any physical entity in the game world.
     * * This class decouples the "Logical Position" (Cartesian) from the
     * "Visual Position" (Isometric).
     * * It handles the Z-ordering depth calculation automatically.
     */
    class CombatWorldObject {
    public:
        CombatWorldObject() = default;
        virtual ~CombatWorldObject() = default;

        // ==============================
        // Core Physics
        // ==============================

/**
         * @brief Sets the logical position in the 2D Cartesian world.
         * @param x The x coordinate (Horizontal).
         * @param y The y coordinate (Vertical).
         */
        void setLogicalPosition(float x, float y) {
            position = sf::Vector2f(x, y);
            collider.position.x = x - (collider.size.x / 2.0f);
            collider.position.y = y - (collider.size.y / 2.0f);
            updateVisuals();
        }
        /**
         * @brief Gets the current logical position.
         * @return sf::Vector2f (x, y)
         */
        sf::Vector2f getLogicalPosition() const {
            return position;
        }

        /**
         * @brief Sets the size of the physical footprint (hitbox).
         * * This should represent the space the object takes up on the FLOOR.
         * @param width Width in logical units.
         * @param height Height in logical units.
         */
        void setColliderSize(float width, float height) {
            collider.size = sf::Vector2f(width, height);
            // Re-center using the current position
            setLogicalPosition(position.x, position.y);
        }

        /**
         * @brief Gets the axis-aligned bounding box of the object.
         * @return sf::FloatRect The hitbox in logical coordinates.
         */
        sf::FloatRect getCollider() const {
            return collider;
        }


        // ==============================
        // Rendering Helpers
        // ==============================

        /**
         * @brief Calculates the screen position for drawing.
         * * Uses the IsoHelpers to convert logical (x,y) to isometric screen (x,y).
         * @return sf::Vector2f The pixel position for the sprite.
         */
        sf::Vector2f getRenderPosition() const {
            return Iso::worldToScreen(position);
        }

        /**
         * @brief Calculates the Z-depth for sorting.
         * * Objects with a higher depth value are drawn LATER (on top).
         * @return float The sum of x + y.
         */
        float getRenderDepth() const {
            return Iso::getRenderDepth(position);
        }
        // ==============================
        // Visuals
        // ==============================

        /**
         * @brief Constructs the sprite inside the optional container.
         */
        void setSprite(const sf::Texture& texture, sf::IntRect rect = sf::IntRect()) {
            // 1. Construct the sprite using emplace (SFML 3 requires Texture in constructor)
            // If rect is provided (size > 0), use the (Texture, Rect) constructor
            if (rect.size.x > 0 && rect.size.y > 0) {
                m_sprite.emplace(texture, rect);
            }
            else {
                // Otherwise use the (Texture) constructor
                m_sprite.emplace(texture);
            }

            // 2. Setup Origin (Bottom Center for Isometric)
            sf::FloatRect bounds = m_sprite->getLocalBounds();
            m_sprite->setOrigin({ bounds.size.x / 2.f, bounds.size.y });

            // 3. Scale
            m_sprite->setScale({ 2.0f, 2.0f });

            updateVisuals();
        }

        /**
         * @brief Updates the sprite's screen position based on the logical position.
         * Call this in your update loop or right before drawing.
         */
        void updateVisuals() {
            // Only update if the sprite actually exists
            if (m_sprite.has_value()) {
                sf::Vector2f screenPos = Iso::worldToScreen(position);
                m_sprite->setPosition(screenPos);
            }
        }
        /**
         * @return A pointer to the sprite, or nullptr if not yet set.
         */
        const sf::Sprite* getSprite() const {
            if (m_sprite.has_value()) {
                return &m_sprite.value();
            }
            return nullptr;
        }

        // ==============================
        // State Properties
        // ==============================

        virtual bool blocksMovement() const { return true; }
        virtual bool blocksSight() const { return false; }
        virtual std::string getName() const { return "Object"; }

    protected:
        sf::Vector2f position;   ///< The center point of the object in the world.
        sf::FloatRect collider;  ///< The physical footprint.
        std::optional<sf::Sprite> m_sprite;
    };
}