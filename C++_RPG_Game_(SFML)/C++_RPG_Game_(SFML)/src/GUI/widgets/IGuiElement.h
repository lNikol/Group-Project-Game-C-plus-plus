#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <optional>

namespace RPG {

    /**
     * @brief Abstract base class for all interactive User Interface elements.
     * * Combines sf::Drawable (rendering), sf::Transformable (positioning/scaling),
     * and custom event handling logic.
     * * Supports hover detection and tooltip text storage.
     */
    class IGuiElement : public sf::Drawable, public sf::Transformable {
    public:
        virtual ~IGuiElement() = default;

        /**
         * @brief Updates the element's logic (animations, timers, etc.).
         * @param dt Delta time in seconds.
         */
        virtual void update(float dt) = 0;

        /**
         * @brief Processes input events (mouse clicks, movement, etc.).
         * * @param window The window reference (used for coordinate mapping).
         * @param event The specific SFML event to process.
         * @return true if the event was consumed by this element (stops propagation), false otherwise.
         */
        virtual bool handleEvent(const sf::RenderWindow& window, const sf::Event& event) {
            return false;
        }

        /**
         * @brief Sets the text to be displayed when the mouse hovers over this element.
         * @param text The tooltip string.
         */
        void setTooltipText(const std::string& text) { m_tooltipText = text; }

        /**
         * @brief Returns the tooltip text only if the element is currently hovered.
         * @return std::optional<std::string> The text if hovered and set, otherwise std::nullopt.
         */
        std::optional<std::string> getTooltipIfHovered() const {
            if (m_isHovered && !m_tooltipText.empty()) {
                return m_tooltipText;
            }
            return std::nullopt;
        }

        /**
         * @brief Calculates the world-space bounding box of the element.
         * * Must be implemented by derived classes to account for custom shapes.
         */
        virtual sf::FloatRect getGlobalBounds() const = 0;

    protected:
        bool m_isHovered = false;
        std::string m_tooltipText;

        /**
         * @brief Helper to check if the mouse is inside the element's bounds.
         * * Updates the m_isHovered flag.
         * * @param window The render window.
         * @param mousePosPixel The raw pixel coordinates of the mouse.
         */
        void updateHoverState(const sf::RenderWindow& window, const sf::Vector2i& mousePosPixel) {
            sf::Vector2f mousePos = window.mapPixelToCoords(mousePosPixel);

            if (getGlobalBounds().contains(mousePos)) {
                m_isHovered = true;
            }
            else {
                m_isHovered = false;
            }
        }
    };
}