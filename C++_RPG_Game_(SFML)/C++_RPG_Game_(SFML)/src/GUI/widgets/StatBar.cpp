#include "StatBar.h"
#include <algorithm>

namespace RPG {

    StatBar::StatBar(const sf::Vector2f& size, sf::Color baseColor)
        : m_size(size),
        m_baseColor(baseColor),
        m_backgroundVertices(sf::PrimitiveType::TriangleStrip, 4),
        m_coloredVertices(sf::PrimitiveType::TriangleStrip, 4)
    {
        //"Empty" background color (Dark Grey/Black glass)
        sf::Color bgDark(20, 20, 20, 150);
        sf::Color bgLight(50, 50, 50, 150);


        // Top-Left
        m_backgroundVertices[0].position = { 0.f, 0.f };
        m_backgroundVertices[0].color = bgDark;

        // Bottom-Left
        m_backgroundVertices[1].position = { 0.f, m_size.y };
        m_backgroundVertices[1].color = bgDark;

        // Top-Right
        m_backgroundVertices[2].position = { m_size.x, 0.f };
        m_backgroundVertices[2].color = bgLight;

        // Bottom-Right
        m_backgroundVertices[3].position = { m_size.x, m_size.y };
        m_backgroundVertices[3].color = bgDark;

        updateGeometry();
    }

    void StatBar::setValues(float current, float max) {
        m_currentVal = current;
        m_maxVal = max;

        setTooltipText(std::to_string((int)m_currentVal) + " / " + std::to_string((int)m_maxVal));

        updateGeometry();
    }

    void StatBar::updateGeometry() {
        float ratio = m_currentVal / m_maxVal;
        if (ratio < 0.f) ratio = 0.f;
        if (ratio > 1.f) ratio = 1.f;

        float liquidHeight = m_size.y * ratio;
        float emptySpace = m_size.y - liquidHeight;

        // Filled Gradient Colors
        sf::Color dark = sf::Color(m_baseColor.r / 2, m_baseColor.g / 2, m_baseColor.b / 2);
        sf::Color light = sf::Color(std::min(255, m_baseColor.r + 50),
            std::min(255, m_baseColor.g + 50),
            std::min(255, m_baseColor.b + 50));


        // Top-Left
        m_coloredVertices[0].position = { 0.f, emptySpace };
        m_coloredVertices[0].color = dark;

        // Bottom-Left
        m_coloredVertices[1].position = { 0.f, m_size.y };
        m_coloredVertices[1].color = dark;

        // Top-Right
        m_coloredVertices[2].position = { m_size.x, emptySpace };
        m_coloredVertices[2].color = light; // The shiny part of the glass

        // Bottom-Right
        m_coloredVertices[3].position = { m_size.x, m_size.y };
        m_coloredVertices[3].color = dark;
    }

    sf::FloatRect StatBar::getGlobalBounds() const {
        sf::Vector2f topLeft = { 0.f, 0.f };
        sf::Vector2f topRight = { m_size.x, 0.f };
        sf::Vector2f bottomRight = { m_size.x, m_size.y };
        sf::Vector2f bottomLeft = { 0.f, m_size.y };

        const sf::Transform& transform = getTransform();

        sf::Vector2f p1 = transform.transformPoint(topLeft);
        sf::Vector2f p2 = transform.transformPoint(topRight);
        sf::Vector2f p3 = transform.transformPoint(bottomRight);
        sf::Vector2f p4 = transform.transformPoint(bottomLeft);

        float minX = std::min({ p1.x, p2.x, p3.x, p4.x });
        float maxX = std::max({ p1.x, p2.x, p3.x, p4.x });
        float minY = std::min({ p1.y, p2.y, p3.y, p4.y });
        float maxY = std::max({ p1.y, p2.y, p3.y, p4.y });

        return sf::FloatRect({ minX, minY }, { maxX - minX, maxY - minY });
    }

    bool StatBar::handleEvent(sf::RenderWindow& window, const sf::Event& event) {
        if (const auto* mouseEvent = event.getIf<sf::Event::MouseMoved>()) {
            updateHoverState(window, mouseEvent->position);
        }

        return false;
    }

    void StatBar::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        // Apply transform
        states.transform *= getTransform();

        // Draw Background
        target.draw(m_backgroundVertices, states);

        // Draw Filling
        target.draw(m_coloredVertices, states);
    }
}