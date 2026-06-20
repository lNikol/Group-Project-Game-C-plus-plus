#include "NineSlice.h"
#include <cmath>

namespace RPG {

    NineSlice::NineSlice(const sf::Texture& texture, int cornerSize, float scale) {
        setTexture(texture, cornerSize, scale);
    }

    void NineSlice::setTexture(const sf::Texture& texture, int cornerSize, float scale) {
        m_texture = &texture;
        m_cornerSize = cornerSize;
        m_scale = scale;

        sf::Vector2u texSize = m_texture->getSize();
        if (m_size.x == 0 && m_size.y == 0) {
            setSize(sf::Vector2f(static_cast<float>(texSize.x) * m_scale, static_cast<float>(texSize.y) * m_scale));
        } else {
            updateGeometry();
        }
    }

    void NineSlice::setSize(const sf::Vector2f& size) {
        m_size = size;
        updateGeometry();
    }

    sf::Vector2f NineSlice::getSize() const {
        return m_size;
    }

    sf::FloatRect NineSlice::getLocalBounds() const {
        return sf::FloatRect({0.f, 0.f}, m_size);
    }

    void NineSlice::updateGeometry() {
        if (!m_texture) return;

        m_vertices.clear();

        // Screen sizes
        float cs = static_cast<float>(m_cornerSize) * m_scale;
        float midW = m_size.x - 2.f * cs;
        float midH = m_size.y - 2.f * cs;

        if (midW < 0) midW = 0;
        if (midH < 0) midH = 0;

        // Texture sizes
        sf::Vector2u texSize = m_texture->getSize();
        float tw = static_cast<float>(texSize.x);
        float th = static_cast<float>(texSize.y);
        float tcs = static_cast<float>(m_cornerSize);

        // X coords (positions)
        float px[4] = { 0.f, cs, cs + midW, m_size.x };
        // Y coords (positions)
        float py[4] = { 0.f, cs, cs + midH, m_size.y };

        // X coords (texture)
        float tx[4] = { 0.f, tcs, tw - tcs, tw };
        // Y coords (texture)
        float ty[4] = { 0.f, tcs, th - tcs, th };

        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                // Top-Left
                sf::Vertex v1(sf::Vector2f(px[x], py[y]), sf::Color::White, sf::Vector2f(tx[x], ty[y]));
                // Top-Right
                sf::Vertex v2(sf::Vector2f(px[x + 1], py[y]), sf::Color::White, sf::Vector2f(tx[x + 1], ty[y]));
                // Bottom-Right
                sf::Vertex v3(sf::Vector2f(px[x + 1], py[y + 1]), sf::Color::White, sf::Vector2f(tx[x + 1], ty[y + 1]));
                // Bottom-Left
                sf::Vertex v4(sf::Vector2f(px[x], py[y + 1]), sf::Color::White, sf::Vector2f(tx[x], ty[y + 1]));

                m_vertices.append(v1);
                m_vertices.append(v2);
                m_vertices.append(v3);

                m_vertices.append(v1);
                m_vertices.append(v3);
                m_vertices.append(v4);
            }
        }
    }

    void NineSlice::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        if (!m_texture) return;
        states.transform *= getTransform();
        states.texture = m_texture;
        target.draw(m_vertices, states);
    }
}
