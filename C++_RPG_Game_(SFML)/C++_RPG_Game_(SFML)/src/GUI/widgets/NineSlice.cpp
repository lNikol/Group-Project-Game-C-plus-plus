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

    void NineSlice::setColor(const sf::Color& color) {
        m_color = color;
        // Update colors of all existing vertices
        for (std::size_t i = 0; i < m_vertices.getVertexCount(); ++i) {
            m_vertices[i].color = color;
        }
    }

    const sf::Color& NineSlice::getColor() const {
        return m_color;
    }

    void NineSlice::updateGeometry() {
        if (!m_texture) return;

        m_vertices.clear();

        // Screen sizes - gracefully clamp if the requested size is smaller than the corners
        float csX = std::min(m_size.x / 2.f, static_cast<float>(m_cornerSize) * m_scale);
        float csY = std::min(m_size.y / 2.f, static_cast<float>(m_cornerSize) * m_scale);
        
        float midW = m_size.x - 2.f * csX;
        float midH = m_size.y - 2.f * csY;

        if (midW < 0) midW = 0;
        if (midH < 0) midH = 0;

        // Texture sizes
        sf::Vector2u texSize = m_texture->getSize();
        float tw = static_cast<float>(texSize.x);
        float th = static_cast<float>(texSize.y);
        
        // Texture clip sizes for corners (if window size < corner size)
        float tcsX = csX / m_scale;
        float tcsY = csY / m_scale;
        
        // The fixed middle section of the texture
        float tcs = static_cast<float>(m_cornerSize);
        float texMidW = tw - 2.f * tcs;
        float texMidH = th - 2.f * tcs;

        // Tiling Lambda Function
        auto addTiledRegion = [&](float destX, float destY, float destW, float destH, float srcX, float srcY, float srcW, float srcH) {
            if (destW <= 0 || destH <= 0 || srcW <= 0 || srcH <= 0) return;

            float scaledSrcW = srcW * m_scale;
            float scaledSrcH = srcH * m_scale;

            for (float y = 0; y < destH; y += scaledSrcH) {
                float h = std::min(scaledSrcH, destH - y);
                float th_clip = h / m_scale;

                for (float x = 0; x < destW; x += scaledSrcW) {
                    float w = std::min(scaledSrcW, destW - x);
                    float tw_clip = w / m_scale;

                    sf::Vertex v1(sf::Vector2f(destX + x, destY + y), m_color, sf::Vector2f(srcX, srcY));
                    sf::Vertex v2(sf::Vector2f(destX + x + w, destY + y), m_color, sf::Vector2f(srcX + tw_clip, srcY));
                    sf::Vertex v3(sf::Vector2f(destX + x + w, destY + y + h), m_color, sf::Vector2f(srcX + tw_clip, srcY + th_clip));
                    sf::Vertex v4(sf::Vector2f(destX + x, destY + y + h), m_color, sf::Vector2f(srcX, srcY + th_clip));

                    m_vertices.append(v1);
                    m_vertices.append(v2);
                    m_vertices.append(v3);

                    m_vertices.append(v1);
                    m_vertices.append(v3);
                    m_vertices.append(v4);
                }
            }
        };

        // Top-Left
        addTiledRegion(0.f, 0.f, csX, csY, 0.f, 0.f, tcsX, tcsY);
        // Top
        addTiledRegion(csX, 0.f, midW, csY, tcs, 0.f, texMidW, tcsY);
        // Top-Right
        addTiledRegion(csX + midW, 0.f, csX, csY, tw - tcsX, 0.f, tcsX, tcsY);

        // Left
        addTiledRegion(0.f, csY, csX, midH, 0.f, tcs, tcsX, texMidH);
        // Center
        addTiledRegion(csX, csY, midW, midH, tcs, tcs, texMidW, texMidH);
        // Right
        addTiledRegion(csX + midW, csY, csX, midH, tw - tcsX, tcs, tcsX, texMidH);

        // Bottom-Left
        addTiledRegion(0.f, csY + midH, csX, csY, 0.f, th - tcsY, tcsX, tcsY);
        // Bottom
        addTiledRegion(csX, csY + midH, midW, csY, tcs, th - tcsY, texMidW, tcsY);
        // Bottom-Right
        addTiledRegion(csX + midW, csY + midH, csX, csY, tw - tcsX, th - tcsY, tcsX, tcsY);
    }

    void NineSlice::draw(sf::RenderTarget& target, sf::RenderStates states) const {
        if (!m_texture) return;
        states.transform *= getTransform();
        states.texture = m_texture;
        target.draw(m_vertices, states);
    }
}
