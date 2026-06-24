#include "Tooltip.h"
#include "worldmap/AssetManager.h"
#include <algorithm>

namespace RPG {

    Tooltip::Tooltip(const sf::Font& font)
        : m_text(font)
    {
        // 1. Text Setup
        m_text.setCharacterSize(16);
        m_text.setFillColor(sf::Color::White);

        // 2. Background Setup
        if (const sf::Texture* tex = AssetManager::getInstance().getTexture("ui_window")) {
            m_background.emplace(*tex, 32, 0.5f); // 32px corners scaled by 0.5f
        }
    }

    void Tooltip::update(const std::string& textStr, sf::Vector2f mousePos, sf::Vector2f viewSize) {
        if (textStr.empty()) {
            m_isVisible = false;
            return;
        }

        m_isVisible = true;
        m_text.setString(textStr);

        // 1. Resize Background based on Text
        sf::FloatRect textBounds = m_text.getLocalBounds();
        sf::Vector2f boxSize(
            std::max(32.f, textBounds.size.x + (PADDING * 2.f)),
            std::max(32.f, textBounds.size.y + (PADDING * 2.f) + 4.f)
        );
        if (m_background) {
            m_background->setSize(boxSize);
        }

        // 2. Calculate Position (Smart Clamping)
        // Start offset from mouse
        sf::Vector2f drawPos = mousePos + sf::Vector2f(OFFSET, OFFSET);

        // Flip Horizontal if going off right edge
        if (drawPos.x + boxSize.x > viewSize.x) {
            drawPos.x = mousePos.x - boxSize.x - 5.f; // 5px padding from mouse
        }

        // Flip Vertical if going off bottom edge
        if (drawPos.y + boxSize.y > viewSize.y) {
            drawPos.y = mousePos.y - boxSize.y - 5.f;
        }

        // 3. Update Visuals
        if (m_background) {
            m_background->setPosition(drawPos);
        }

        // Align text inside the box (accounting for font vertical quirks)
        m_text.setPosition({
            drawPos.x + PADDING - textBounds.position.x,
            drawPos.y + PADDING - textBounds.position.y
            });
    }

    void Tooltip::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        if (m_isVisible) {
            if (m_background) {
                target.draw(*m_background, states);
            }
            target.draw(m_text, states);
        }
    }
}