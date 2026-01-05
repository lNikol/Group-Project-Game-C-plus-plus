#pragma once
#include "IGuiElement.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <format>

namespace RPG {

    class StatBar : public IGuiElement {
    public:
        StatBar(const sf::Vector2f& size, sf::Color baseColor);

        void setValues(float current, float max);

        void update(float dt) override {}

        bool handleEvent(const sf::RenderWindow& window, const sf::Event& event) override;

        sf::FloatRect getGlobalBounds() const override;

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void updateGeometry();

        sf::Vector2f m_size;
        sf::Color m_baseColor;

        float m_currentVal = 100.f;
        float m_maxVal = 100.f;

        sf::VertexArray m_backgroundVertices; 
        sf::VertexArray m_coloredVertices;
    };
}
