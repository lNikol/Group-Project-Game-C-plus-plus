#pragma once
#include "GUI/windows/WindowBase.h"
#include "GUI/widgets/ActionSlot.h"
#include "GUI/widgets/Tooltip.h"
#include "combat/PartyData.h"
#include "combat/CombatAbility.h"
#include <map>
#include <vector>

namespace RPG {

    /**
     * @class InventoryWindow
     * @brief A unified UI window that manages both the party's shared inventory and individual character equipment.
     * * @details The window is divided into two main sections:
     * - Left Pane: Character Sheet (displays currently selected character, their equipped items, and calculated stats).
     * - Right Pane: Shared Inventory (the party's common pool of items).
     * It fully supports Drag and Drop functionality for equipping, unequipping, and rearranging items.
     */
    class InventoryWindow : public WindowBase {
    public:
        /**
         * @brief Constructs the inventory window.
         * @param iconSet The master spritesheet containing item and ability icons.
         * @param font The font used for rendering text (character names, stats, buttons).
         */
        InventoryWindow(const Spritesheet& iconSet, const sf::Font& font);

        /**
         * @brief Updates the logic and visual state of the inventory slots.
         * Synchronizes the UI with the underlying PartyData.
         * @param dt Delta time in seconds.
         */
        void update(float dt) override;

        /**
         * @brief Processes input events such as mouse movement and clicks.
         * Handles character switching and the Drag & Drop item mechanics.
         * @param window The game's render window.
         * @param event The SFML event to process.
         * @return True if the event was consumed by the window, false otherwise.
         */
        bool handleEvent(sf::RenderWindow& window, const sf::Event& event) override;
        void setCombatMode(bool isCombat) { m_isCombatMode = isCombat; }
    protected:
        /**
         * @brief Renders the window, its contents, and the dragged item icon if applicable.
         * @param target The render target.
         * @param states Current render states.
         */
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        const sf::Font& m_font;         ///< Reference to the main UI font.
        const Spritesheet& m_iconSet;   ///< Reference to the item icons spritesheet.
        size_t m_selectedCharIndex = 0; ///< The index of the currently displayed character in the active party.

        // ==============================
        // UI Elements
        // ==============================
        std::vector<ActionSlot*> m_bagSlots;                 ///< Pointers to the shared inventory slots.
        std::map<EquipSlot, ActionSlot*> m_equipSlots;       ///< Pointers to the equipment slots mapped by their specific type.

        sf::Text m_charNameText;                             ///< Text displaying the current character's name.
        sf::Text m_prevBtn;                                  ///< Button to cycle to the previous character.
        sf::Text m_nextBtn;                                  ///< Button to cycle to the next character.
        sf::FloatRect m_prevHitbox;
        sf::FloatRect m_nextHitbox;
        bool m_prevHovered = false;                          ///< Hover state for the previous button.
        bool m_nextHovered = false;                          ///< Hover state for the next button.
        bool m_isCombatMode = true;
        sf::Vector2f m_lastMousePos;
        mutable Tooltip m_tooltip;
        std::vector<sf::Text> m_statsTexts;                  ///< Dynamic text objects for displaying calculated stats.

        // ==============================
        // Drag & Drop State
        // ==============================
        std::shared_ptr<IAbility> m_draggedItem = nullptr;   ///< The item currently being dragged (if any).
        sf::Sprite m_draggedIcon;                            ///< The visual representation of the dragged item following the cursor.
        ActionSlot* m_dragSourceSlot = nullptr;              ///< The slot from which the item was initially picked up.
        int m_dragSourceBagIndex = -1;                       ///< The index in the shared inventory (if picked from the bag).
        EquipSlot m_dragSourceEquipSlot = EquipSlot::None;   ///< The equipment slot type (if picked from the character sheet).

        // ==============================
        // Internal Mechanics
        // ==============================

        /**
         * @brief Initializes the layout, creating all slots and text elements.
         */
        void buildLayout();

        /**
         * @brief Re-evaluates and updates the displayed stats and equipment for the currently selected character.
         */
        void refreshCharacterDisplay();

        /**
         * @brief Cycles the currently displayed character.
         * @param direction 1 for the next character, -1 for the previous character.
         */
        void switchCharacter(int direction);

        /**
         * @brief Calculates the total stats of a character, including base stats and equipment modifiers.
         * @param profile The character profile to calculate stats for.
         * @return The final, combined unit stats.
         */
        UnitStats calculateProfileStats(const CharacterProfile& profile) const;

        /**
         * @brief Initiates the drag operation if a valid item is clicked.
         * @param mousePos The world coordinates of the mouse cursor.
         */
        void handleDragStart(sf::Vector2f mousePos);

        /**
         * @brief Finalizes the drag operation, swapping items if dropped on a valid slot.
         * @param mousePos The world coordinates of the mouse cursor.
         */
        void handleDragEnd(sf::Vector2f mousePos);
    };
}