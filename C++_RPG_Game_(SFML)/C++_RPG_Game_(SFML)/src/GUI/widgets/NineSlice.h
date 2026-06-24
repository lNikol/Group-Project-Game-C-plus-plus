#pragma once
#include <SFML/Graphics.hpp>
#include <array>

namespace RPG {
    /**
     * @class NineSlice
     * @brief A UI element that intelligently scales a texture by keeping its corners intact.
     */
    class NineSlice : public sf::Drawable, public sf::Transformable {
    public:
        NineSlice() = default;

        /**
         * @brief Construct a new Nine Slice object
         * @param texture The texture containing the 9-patch image
         * @param cornerSize The size of the corners in pixels (e.g., 16 for a 48x48 image divided 3x3)
         */
        NineSlice(const sf::Texture& texture, int cornerSize, float scale = 1.0f);

        void setTexture(const sf::Texture& texture, int cornerSize, float scale = 1.0f);

        void setSize(const sf::Vector2f& size);
        sf::Vector2f getSize() const;
        sf::FloatRect getLocalBounds() const;

        void setColor(const sf::Color& color);
        const sf::Color& getColor() const;

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void updateGeometry();

        const sf::Texture* m_texture = nullptr;
        int m_cornerSize = 0;
        float m_scale = 1.0f;
        sf::Vector2f m_size;
        sf::Color m_color = sf::Color::White;

        sf::VertexArray m_vertices{sf::PrimitiveType::Triangles};
    };
}
