#pragma once
#include <SFML/Graphics.hpp>
#include <string>

namespace RPG {

    class Tooltip : public sf::Drawable {
    public:
        explicit Tooltip(const sf::Font& font);
        void update(const std::string& text, sf::Vector2f mousePos, sf::Vector2f viewSize);

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        sf::Text m_text;
        sf::RectangleShape m_background;

        bool m_isVisible = false;

        const float PADDING = 5.f;
        const float OFFSET = 15.f;
    };
}