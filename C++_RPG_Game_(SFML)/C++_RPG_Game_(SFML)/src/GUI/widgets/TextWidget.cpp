#include "TextWidget.h"

namespace RPG {
    TextWidget::TextWidget(const sf::Font& font, const std::string& text, unsigned int charSize, sf::Color color)
        : m_text(font)
    {
        m_text.setString(text);
        m_text.setCharacterSize(charSize);
        m_text.setFillColor(color);
    }

    void TextWidget::setText(const std::string& text) {
        m_text.setString(text);
    }

    void TextWidget::setFillColor(sf::Color color) {
        m_text.setFillColor(color);
    }

    sf::FloatRect TextWidget::getGlobalBounds() const {
        return getAbsoluteTransform().transformRect(m_text.getLocalBounds());
    }

    void TextWidget::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        states.transform *= getTransform();
        target.draw(m_text, states);
    }
}