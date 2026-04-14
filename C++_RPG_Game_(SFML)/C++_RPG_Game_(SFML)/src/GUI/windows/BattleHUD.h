#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "GUI/widgets/StatBar.h"
#include "GUI/widgets/Tooltip.h"
#include "GUI/widgets/ActionSlot.h"
#include "GUI/windows/WindowBase.h"
#include "interfaces/ICombatActor.h"
#include "core/Constants.h"

namespace RPG {

    /**
     * @brief The master controller for the player's heads-up display.
     * * It manages the layout, rendering, and event handling for:
     * - Vitals Bars (HP/MP/Stamina)
     * - Action Bar (Skills)
     * - System Buttons (Inventory/Journal)
     * - Floating Windows
     * - Tooltips
     */
    class BattleHUD : public sf::Drawable {
    public:
        BattleHUD(const Spritesheet& iconSet, const sf::Font& font);

        /**
         * @brief Links a specific actor (usually the player) to the HUD.
         * * The HUD will pull data from this actor every frame to update the UI.
         */
        void setCombatActor(ICombatActor* actor);

        /**
         * @brief Handles mouse events.
         * * Converts window coordinates to UI View coordinates.
         * * Passes events to Windows -> Buttons -> Slots -> Vitals (in that order).
         */
        void handleEvent(sf::RenderWindow& window, const sf::Event& event);

        /**
         * @brief Recalculates the position of all UI elements.
         * * Should be called whenever the window is resized to keep elements anchored
         * (e.g., Vitals bottom-left, Buttons bottom-right).
         */
        void onResize(const sf::Vector2u& newSize);

        /**
         * @brief Syncs UI state with the Actore data.
         * * Updates cooldowns, charges, and vital bar percentages.
         */
        void update(float dt);

        bool isMouseOverUI() const;

        void closeAllWindows();

    protected:
        /**
         * @brief Renders the UI.
         * * Temporarily switches the RenderTarget's view to m_uiView to ensure
         * the UI draws over the game world and ignores camera movement.
         */
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        /**
         * @brief Helper to determine which tooltip string should be displayed.
         * * Checks all UI elements in Z-order (Windows -> Buttons -> Grid -> Vitals).
         */
        std::string resolveTooltipText() const;

    private:
        // Resources
        const Spritesheet& m_iconSet;
        const sf::Font& m_font;

        // Data Source
        ICombatActor* m_actor = nullptr;

        // Vitals
        std::unique_ptr<StatBar> m_hpBar;
        std::unique_ptr<StatBar> m_mpBar;
        std::unique_ptr<StatBar> m_staminaBar;

        // Action Bar
        std::vector<std::unique_ptr<ActionSlot>> m_actionGrid;

        // System Buttons
        std::unique_ptr<ActionSlot> m_satchelBtn;
        std::unique_ptr<ActionSlot> m_journalBtn;

        // Windows
        std::unique_ptr<WindowBase> m_inventoryWindow; // Using Base Windows for now
        std::unique_ptr<WindowBase> m_journalWindow;

        // Internal State
        sf::View m_uiView;
        sf::Vector2u m_currentWindowSize;

        //Tooltip
        sf::Vector2f m_mousePos;
        mutable Tooltip m_tooltip;
    };
}