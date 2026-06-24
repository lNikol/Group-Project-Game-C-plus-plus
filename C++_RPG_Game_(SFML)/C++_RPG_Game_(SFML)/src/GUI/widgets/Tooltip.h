#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include "NineSlice.h"

namespace RPG {

    /**
     * @brief A floating text box that follows the mouse cursor.
     * * Automatically resizes its background to fit the text content.
     * * Includes "smart clamping" logic to prevent it from being drawn off-screen.
     */
    class Tooltip : public sf::Drawable {
    public:
        explicit Tooltip(const sf::Font& font);

        /**
         * @brief Updates the tooltip's text and position.
         * * If textStr is empty, the tooltip becomes invisible.
         * * @param textStr The content to display.
         * @param mousePos The world position of the mouse cursor.
         * @param viewSize The size of the current view (used for edge clamping).
         */
        void update(const std::string& textStr, sf::Vector2f mousePos, sf::Vector2f viewSize);

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        sf::Text m_text;
        std::optional<NineSlice> m_background;

        bool m_isVisible = false;

        const float PADDING = 15.f;
        const float OFFSET = 15.f;
    };
}