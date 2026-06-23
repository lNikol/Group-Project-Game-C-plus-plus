#pragma once
#include "IGuiElement.h"
#include "Interfaces/IAbility.h"
#include "core/Constants.h"
#include <SFML/Graphics.hpp>
#include <memory>

namespace RPG {

    /**
     * @brief A UI container representing a single skill or item slot.
     * * Displays an icon, cooldown timer, charge count, and selection state.
     * * Handles left-click events to execute the bound Ability.
     */
    class ActionSlot : public IGuiElement {
    public:
        /**
         * @brief Constructs an Action Slot.
         * @param iconSet The sprite sheet containing all ability icons.
         * @param font The font used for cooldown and charge text.
         * @param size The dimensions of the slot (default 32x32).
         */
        ActionSlot(const Spritesheet& iconSet, const sf::Font& font, const sf::Vector2f& size = { 32.f, 32.f });

        /**
         * @brief Binds a specific Ability (skill/spell) to this slot.
         * * Automatically updates the icon and tooltip based on the ability data.
         * @param ability Shared pointer to the ability instance.
         */
        void setAbility(std::shared_ptr<IAbility> ability);

        std::shared_ptr<IAbility> getAbility() const;

        /**
         * @brief Checks if the slot currently has no ability assigned.
         */
        bool isEmpty() const { return m_ability == nullptr; }

        /**
         * @brief Toggles the visual "Selected" green border.
         */
        void setSelected(bool selected);
        bool isSelected() const { return m_isSelected; }

        /**
         * @brief Updates cooldown timers and charge displays.
         */
        void update(float dt) override;

        /**
         * @brief Handles mouse clicks.
         * * Left-clicking a hovered slot executes the bound ability.
         */
        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;

        sf::FloatRect getGlobalBounds() const override;

    protected:
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        // Data
        std::shared_ptr<IAbility> m_ability;
        bool m_isSelected = false;

        // Visual Components
        std::optional<sf::Sprite> m_background;
        sf::Vector2f m_size;
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