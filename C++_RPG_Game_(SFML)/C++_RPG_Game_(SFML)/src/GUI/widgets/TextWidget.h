#pragma once
#include "GUI/Widgets/IGuiElement.h"
#include <SFML/Graphics.hpp>
#include <string>

namespace RPG {
    /**
     * @brief A simple text wrapper that inherits from IGuiElement.
     * Allows raw text to be added to WindowBase child lists.
     */
    class TextWidget : public IGuiElement {
    public:
        TextWidget(const sf::Font& font, const std::string& text, unsigned int charSize = 18, sf::Color color = sf::Color::White);

        void setText(const std::string& text);
        void setFillColor(sf::Color color);

        // IGuiElement Overrides
        void update(float dt) override {}
        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override { return false; } // Text is non-interactive
        sf::FloatRect getGlobalBounds() const override;

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        sf::Text m_text;
    };
}