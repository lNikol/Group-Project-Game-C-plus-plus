#pragma once
#include "../Widgets/IGuiElement.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <string>

namespace RPG {

    class WindowBase : public IGuiElement {
    public:
        WindowBase(const sf::Font& font, const sf::Vector2f& size, const std::string& title);

        // Visibility
        void show();
        void hide();
        void toggle();
        bool isVisible() const { return m_isVisible; }

        void addChild(std::unique_ptr<IGuiElement> widget);

        //Overrides
        void update(float dt) override;
        bool handleEvent(const sf::RenderWindow& window, const sf::Event& event) override;
        sf::FloatRect getGlobalBounds() const override;

        std::optional<std::string> getChildTooltipIfHovered() const;

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        // Visuals
        sf::RectangleShape m_background;
        sf::RectangleShape m_titleBar;
        sf::Text m_titleText;

        // Close Button
        sf::RectangleShape m_closeBtnRect;
        sf::Text m_closeBtnText;
        bool m_closeBtnHovered = false;

        // State
        bool m_isVisible = false;

        // The list of widgets inside this window
        std::vector<std::unique_ptr<IGuiElement>> m_children;
    };
}