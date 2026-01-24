#pragma once
#include "IGuiElement.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <format>

namespace RPG {

    /**
     * @brief A dynamic graphical bar used to represent stats (Health, Mana, Stamina).
     * * Renders using a custom VertexArray to create a gradient "glassy" effect
     * without needing external textures.
     */
    class StatBar : public IGuiElement {
    public:
        /**
         * @brief Constructs the stat bar.
         * @param size The width and height of the bar.
         * @param baseColor The primary color of the filled portion (gradients are calculated automatically).
         */
        StatBar(const sf::Vector2f& size, sf::Color baseColor);

        /**
         * @brief Updates the current fill level of the bar.
         * * Also automatically updates the tooltip to show "Current / Max".
         * * @param current The current value (e.g., current HP).
         * @param max The maximum value (e.g., max HP).
         */
        void setValues(float current, float max);

        void update(float dt) override {}

        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;

        sf::FloatRect getGlobalBounds() const override;

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        /**
         * @brief Recalculates vertex positions and colors based on current values.
         */
        void updateGeometry();

        sf::Vector2f m_size;
        sf::Color m_baseColor;

        float m_currentVal = 100.f;
        float m_maxVal = 100.f;

        // Uses TriangleStrip for efficient gradient rendering
        sf::VertexArray m_backgroundVertices;
        sf::VertexArray m_coloredVertices;
    };
}