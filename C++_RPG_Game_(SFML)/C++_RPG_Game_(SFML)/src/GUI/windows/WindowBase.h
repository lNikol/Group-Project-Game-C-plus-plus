#pragma once
#include "GUI/Widgets/IGuiElement.h"
#include <SFML/Graphics.hpp>
#include "GUI/widgets/NineSlice.h"
#include <vector>
#include <memory>
#include <string>

namespace RPG {

    /**
     * @brief A closeable container for other UI widgets.
     * * Includes a standard background, a title bar, and a 'X' close button.
     * * It manages the lifecycle and event propagation for any child widgets added to it.
     */
    class WindowBase : public IGuiElement {
    public:
        /**
         * @brief Creates a new window.
         * @param font The font used for the title and close button.
         * @param size The initial width and height of the content area.
         * @param title The string displayed in the header.
         */
        WindowBase(const sf::Font& font, const sf::Vector2f& size, const std::string& title);

        // ==============================
        // Visibility & Size Control
        // ==============================
        void show();
        void hide();
        void toggle();
        bool isVisible() const { return m_isVisible; }
        
        virtual void setSize(const sf::Vector2f& size);
        sf::Vector2f getSize() const;

        /**
         * @brief Adds a widget (Button, Slot, Text) to the window.
         * * The window takes ownership of the widget pointer.
         */
        void addChild(std::unique_ptr<IGuiElement> widget);

        // ==============================
        // Overrides
        // ==============================

        /**
         * @brief Updates all child widgets.
         * * Returns early if the window is hidden.
         */
        void update(float dt) override;

        /**
         * @brief Handles events for the window and its children.
         * * Priority:
         * 1. Close Button
         * 2. Child Widgets
         * 3. Window Background (swallows clicks to prevent clicking objects behind the window).
         */
        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;

        sf::FloatRect getGlobalBounds() const override;

        /**
         * @brief Recursively checks if any child widget is hovered and returns its tooltip.
         */
        std::optional<std::string> getChildTooltipIfHovered() const;

    protected:
        /**
         * @brief Draws the window frame, title bar, close button, and all children.
         */
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        // Visuals
        NineSlice m_background;
        // sf::RectangleShape m_titleBar; // Removed
        sf::Text m_titleText;

        // Close Button
        std::optional<sf::Sprite> m_closeBtnSprite;
        bool m_closeBtnHovered = false;

        // State
        bool m_isVisible = false;

        // The list of widgets inside this window
        std::vector<std::unique_ptr<IGuiElement>> m_children;
    };
}