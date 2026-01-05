#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
namespace RPG {
    class IsoGrid : public sf::Drawable, public sf::Transformable {
    public:

        IsoGrid(int rows, int cols, float tileSize, sf::Color baseColor);

        void rebuild(int rows, int cols, float tileSize, sf::Color baseColor);

    private:
        sf::VertexArray m_vertices;

        void addIsoCube(float x, float y, float size, sf::Color color);

        void addQuad(sf::Vector2f p1, sf::Vector2f p2, sf::Vector2f p3, sf::Vector2f p4, sf::Color color);

        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    };
}