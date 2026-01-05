#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <optional>

namespace RPG {

    class IGuiElement : public sf::Drawable, public sf::Transformable {
    public:
        virtual ~IGuiElement() = default;

        virtual void update(float dt) = 0;

        //'true' if this element consumed the event (preventing click-through)
        virtual bool handleEvent(const sf::RenderWindow& window, const sf::Event& event) {
            return false;
        }

        void setTooltipText(const std::string& text) { m_tooltipText = text; }

        std::optional<std::string> getTooltipIfHovered() const {
            if (m_isHovered && !m_tooltipText.empty()) {
                return m_tooltipText;
            }
            return std::nullopt;
        }

        virtual sf::FloatRect getGlobalBounds() const = 0;

    protected:
        bool m_isHovered = false;
        std::string m_tooltipText;


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