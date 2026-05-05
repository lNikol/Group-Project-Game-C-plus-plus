#pragma once
#include <string>
#include <vector>
#include <array>

namespace RPG {

    // ==========================================
    // Core Enums
    // ==========================================

    enum class StatType {
        None = 0,
        Strength,
        Dexterity,
        Intellect,
        Vitality,
        Count     // Automatically tracks the number of stats for our array
    };

    enum class EffectType {
        Damage,
        Heal,
        Buff,
        Debuff,
        Stun
    };

    enum class TargetType {
        Self,
        SingleEnemy,
        SingleAlly,
        AreaEnemy,
        AreaAlly
    };

    // ==========================================
    // Stat Container
    // ==========================================

    /**
     * @brief Array-based stat container for O(1) lookups and easy scaling.
     */
    struct UnitStats {
        std::array<float, static_cast<size_t>(StatType::Count)> values;

        UnitStats() {
            values.fill(10.0f); // Default baseline
            values[static_cast<size_t>(StatType::None)] = 0.0f; // 'None' must always be 0
        }

        // Access via enum: myStats[StatType::Strength]
        float& operator[](StatType type) {
            return values[static_cast<size_t>(type)];
        }

        float operator[](StatType type) const {
            return values[static_cast<size_t>(type)];
        }
    };

    // ==========================================
    // Blueprints
    // ==========================================

    /**
     * @brief Represents one single action an ability performs (The Payload).
     * Includes power scaling, critical hit logic, and duration for Status Effects.
     */
    struct EffectData {
        EffectType type = EffectType::Damage;

        // --- Power Scaling ---
        float baseValue = 0.f;
        StatType scalingStat = StatType::None;
        float scalingFactor = 0.0f;

        // --- Target Modification ---
        StatType targetStat = StatType::None; ///< Which stat to modify if type is Buff or Debuff

        // --- Critical Hit Scaling ---
        float baseCritChance = 0.0f;
        StatType critScalingStat = StatType::None;
        float critScalingFactor = 0.0f;
        float critMultiplier = 1.5f;

        // --- Constraints ---
        int durationTurns = 0;    ///< If > 0, this effect becomes an ActiveStatus (DoT/Buff)
        float applyChance = 1.0f; ///< 1.0 = 100% chance to apply the effect
    };

    /**
     * @brief A shared, read-only definition for a specific skill.
     */
    struct AbilityDefinition {
        std::string id;
        std::string name;
        std::string description;
        int iconIndex = 0;

        TargetType targetType = TargetType::SingleEnemy;
        int cooldownTurns = 1;
        float range = 100.f; // Max cast distance in logical units
        float radius = 0.f;  // AOE Blast Radius (0 = Single Target)

        float manaCost = 0.f;
        float staminaCost = 0.f;

        bool isConsumable = false;
        int maxCharges = -1;
        std::vector<EffectData> effects;


    };
}