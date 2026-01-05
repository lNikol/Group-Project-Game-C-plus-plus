#pragma once
#include "IAbility.h"
#include <vector>
#include <memory>
#include <string>

namespace RPG {

    struct Vitals {
        float hp, maxHp;
        float mp, maxMp;
        float stamina, maxStamina;
    };

    enum class EquipSlot {
        Head, Chest, MainHand, OffHand, Feet, Ring1, Ring2
    };

    class ICombatActor {
    public:
        virtual ~ICombatActor() = default;

        virtual Vitals getVitals() const = 0;
        virtual std::string getName() const = 0;

        // Hotbar
        virtual std::shared_ptr<IAbility> getHotbarAbility(int index) const = 0;
        virtual int getHotbarSize() const = 0;

        // Inventory
        virtual std::shared_ptr<IAbility> getInventoryItem(int index) const = 0;
        virtual int getInventorySize() const = 0;

        // Equipment
        virtual std::shared_ptr<IAbility> getEquipment(EquipSlot slot) const = 0;
    };
}