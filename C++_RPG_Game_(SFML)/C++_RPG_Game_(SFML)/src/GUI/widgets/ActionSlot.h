#pragma once
#include "IGuiElement.h"
#include "../../Interfaces/IAbility.h"
#include "../../core/Constants.h"
#include <SFML/Graphics.hpp>
#include <memory>

namespace RPG {

    class ActionSlot : public IGuiElement {
    public:
        ActionSlot(const Spritesheet& iconSet, const sf::Font& font, const sf::Vector2f& size = { 32.f, 32.f });

        void setAbility(std::shared_ptr<IAbility> ability);
        std::shared_ptr<IAbility> getAbility() const;
        bool isEmpty() const { return m_ability == nullptr; }

        void setSelected(bool selected);
        bool isSelected() const { return m_isSelected; }

        void update(float dt) override;
        bool handleEvent(const sf::RenderWindow& window, const sf::Event& event) override;
        sf::FloatRect getGlobalBounds() const override;

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:

        // Data
        std::shared_ptr<IAbility> m_ability;
        bool m_isSelected = false;

        // Visual Components
        sf::RectangleShape m_background;
        sf::RectangleShape m_border;     // Selection Highlight
        sf::RectangleShape m_overlay;    // Grey out if !canBeCast
        sf::Sprite m_icon;

        // Text Components
        const sf::Font& m_font;
        sf::Text m_chargesText;
        sf::Text m_cooldownText; 

        // Constants
        static const int ICON_SIZE = 24;
    };
}