#pragma once
#include <string>

namespace RPG {

    /**
     * @brief Interface for all actionable skills, spells, or consumable items.
     * * This abstract class defines the contract for anything that can be placed
     * in an ActionSlot. It separates the "Visuals" (icons/text) from the "Mechanics"
     * (cooldowns/costs) and "Execution" (gameplay logic).
     */
    class IAbility {
    public:
        virtual ~IAbility() = default;

        // ==============================
        // Visuals
        // ==============================

        /**
         * @brief Gets the index of the icon on the master spritesheet.
         * * This index maps to a grid position (e.g., row/column calculation)
         * defined in the ActionSlot or AssetManager logic.
         * @return int The linear index of the sprite.
         */
        virtual int getIconIndex() const = 0;

        /**
         * @brief Gets the description text to display when hovered.
         * @return std::string The tooltip content (e.g., "Fireball: Deals 50 dmg").
         */
        virtual std::string getTooltip() const = 0;


        // ==============================
        // Mechanics
        // ==============================

        /**
         * @brief Checks if the ability conditions are met (Mana cost, Stamina, etc.).
         * * This is used by the UI to visually "grey out" the slot if the player
         * cannot afford the cost, separate from the cooldown state.
         * @return true If the player has enough resources to cast.
         */
        virtual bool canBeCast() const = 0;

        /**
         * @brief Gets the number of remaining uses (for items/consumables).
         * @return int The current charge count, or -1 if the ability has infinite uses.
         */
        virtual int getCharges() const = 0;

        /**
         * @brief Gets the remaining cooldown time.
         * @return int The turns remaining, or 0 if ready.
         */
        virtual int getCooldown() const = 0;


        // ==============================
        // Execution
        // ==============================

        /**
         * @brief Triggers the ability's gameplay effect.
         * * This is called when the ActionSlot is clicked. Implementations should
         * handle resource deduction, projectile spawning, or stat changes here.
         */
        virtual void execute() = 0;
    };
}