#include "IsoGrid.h"
namespace RPG {
    IsoGrid::IsoGrid(int rows, int cols, float tileSize, sf::Color baseColor) {
        m_vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
        rebuild(rows, cols, tileSize, baseColor);
    }

    void IsoGrid::rebuild(int rows, int cols, float tileSize, sf::Color baseColor) {
        m_vertices.clear();

        const float sqrt3 = std::sqrt(3.0f);
        float width = tileSize * sqrt3;
        float height = tileSize * 2.0f;

        float xSpacing = width;
        float ySpacing = height * 0.75f;

        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                float posX = x * xSpacing;
                float posY = y * ySpacing;

                if (y % 2 != 0) {
                    posX += width / 2.0f;
                }

                sf::Color tileColor = baseColor;
                if ((x + y) % 2 == 0) {
                    // Slightly darken alternate tiles
                    tileColor.r = static_cast<uint8_t>(std::max(0, tileColor.r - 10));
                    tileColor.g = static_cast<uint8_t>(std::max(0, tileColor.g - 10));
                    tileColor.b = static_cast<uint8_t>(std::max(0, tileColor.b - 10));
                }

                addIsoCube(posX, posY, tileSize, tileColor);
            }
        }
    }

    void IsoGrid::addIsoCube(float x, float y, float size, sf::Color baseColor) {
        // 30 degrees
        sf::Angle angle = sf::degrees(30.f);

        float dx = size * std::cos(angle.asRadians());
        float dy = size * std::sin(angle.asRadians());

        // Calculate the 7 key vertices
        sf::Vector2f center(x, y);
        sf::Vector2f top(x, y - size);
        sf::Vector2f bottom(x, y + size);

        sf::Vector2f topLeft(x - dx, y - dy);
        sf::Vector2f topRight(x + dx, y - dy);
        sf::Vector2f bottomLeft(x - dx, y + dy);
        sf::Vector2f bottomRight(x + dx, y + dy);

        // --- FACE 1: TOP (Brightest) ---
        sf::Color topColor = baseColor;
        topColor.r = std::min(255, topColor.r + 40);
        topColor.g = std::min(255, topColor.g + 40);
        topColor.b = std::min(255, topColor.b + 40);
        addQuad(center, topLeft, top, topRight, topColor);

        // --- FACE 2: LEFT (Medium Shadow) ---
        sf::Color leftColor = baseColor;
        leftColor.r = std::max(0, leftColor.r - 20);
        leftColor.g = std::max(0, leftColor.g - 20);
        leftColor.b = std::max(0, leftColor.b - 20);
        addQuad(center, topLeft, bottomLeft, bottom, leftColor);

        // --- FACE 3: RIGHT (Darkest Shadow) ---
        sf::Color rightColor = baseColor;
        rightColor.r = std::max(0, rightColor.r - 60);
        rightColor.g = std::max(0, rightColor.g - 60);
        rightColor.b = std::max(0, rightColor.b - 60);
        addQuad(center, bottom, bottomRight, topRight, rightColor);
    }

    void IsoGrid::addQuad(sf::Vector2f p1, sf::Vector2f p2, sf::Vector2f p3, sf::Vector2f p4, sf::Color color) {
        // Triangle 1: p1-p2-p3
        m_vertices.append(sf::Vertex{ p1, color });
        m_vertices.append(sf::Vertex{ p2, color });
        m_vertices.append(sf::Vertex{ p3, color });

        // Triangle 2: p3-p4-p1
        m_vertices.append(sf::Vertex{ p3, color });
        m_vertices.append(sf::Vertex{ p4, color });
        m_vertices.append(sf::Vertex{ p1, color });
    }

    void IsoGrid::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        // Apply the transform
        states.transform *= getTransform();

        // Draw the vertex array using the modified states
        target.draw(m_vertices, states);
    }
}