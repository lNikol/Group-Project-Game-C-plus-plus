#pragma once
#include "AbilityData.h"

namespace RPG {

    class Unit;
    class BattleManager;

    /**
     * @brief Static helper class to calculate and apply ability effects.
     */
    class AbilityProcessor {
    public:
        /**
         * @brief Calculates the final power: Base + (CasterStat * ScalingFactor)
         */
        static float calculatePower(const EffectData& effect, const Unit& caster);

        /**
         * @brief Calculates the final critical hit chance for this specific effect.
         * Formula: Innate Caster Crit + Effect Base Crit + (CasterStat * CritScalingFactor)
         */
        static float calculateCritChance(const EffectData& effect, const Unit& caster);

        /**
         * @brief Resolves RNG and applies a specific effect to a target.
         * Will apply instantly OR package it into an ActiveEffect if duration > 0.
         */
        static void applyEffect(const EffectData& effect, Unit& source, Unit& target, BattleManager& manager);

        /**
         * @brief Iterates through all effects in a definition and applies them.
         */
        static void processAbility(const AbilityDefinition& abilityDef, Unit& source, Unit& target, BattleManager& manager);
    };
}