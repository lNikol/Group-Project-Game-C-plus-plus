#pragma once
#include "IAbility.h"
#include "combat/AbilityData.h" // Needed for UnitStats
#include <vector>
#include <memory>
#include <string>

namespace RPG {

    /**
     * @brief Simple struct containing current and max values for actor resources.
     */
    struct Vitals {
        float hp, maxHp;
        float mp, maxMp;
        float stamina, maxStamina;
    };

    enum class EquipSlot {
        Head, Chest, MainHand, OffHand, Feet, Ring1, Ring2
    };

    /**
     * @brief Interface for any entity (Player, Enemy, NPC) that can interact with the Combat System.
     * * The BattleHUD relies on this interface to pull data (HP, Skills, Inventory)
     * without needing to know the specific implementation of the Player class.
     */
    class ICombatActor {
    public:
        virtual ~ICombatActor() = default;

        virtual Vitals getVitals() const = 0;
        virtual std::string getName() const = 0;

        /**
         * @brief Retrieves the unit's dynamic stats (Base + Buffs - Debuffs).
         * @return UnitStats The array container of all stats.
         */
        virtual UnitStats getStats() const = 0;

        // ==============================
        // Hotbar
        // ==============================

        /**
         * @brief Retrieves the ability assigned to a specific hotbar slot.
         * @param index The slot index (0-based).
         * @return std::shared_ptr<IAbility> The ability, or nullptr if empty.
         */
        virtual std::shared_ptr<IAbility> getHotbarAbility(uint8_t index) const = 0;
        virtual uint8_t getHotbarSize() const = 0;

        // ==============================
        // Inventory
        // ==============================

        /**
         * @brief Retrieves the item at a specific inventory index.
         * * Note: In this system, Items are treated as IAbility (consumables/equipables).
         */
        virtual std::shared_ptr<IAbility> getInventoryItem(uint8_t index) const = 0;
        virtual uint8_t getInventorySize() const = 0;

        // ==============================
        // Equipment
        // ==============================

        virtual std::shared_ptr<IAbility> getEquipment(EquipSlot slot) const = 0;
    };
}