#pragma once
#include "AbilityData.h"

namespace RPG {

    class Unit; // Forward declaration

    /**
     * @struct ActiveEffect
     * @brief Represents an ongoing effect applied to a unit (DoT, HoT, Buff, Debuff, Stun).
     * * Stores the pre-calculated power so that if the caster dies or gets debuffed
     * later, the poison/buff doesn't suddenly change in strength.
     */
    struct ActiveEffect {
        EffectData baseEffect;   ///< The original blueprint of the effect.
        float lockedPower = 0.f; ///< The exact numerical strength, calculated at the time of cast.
        int turnsRemaining = 0;  ///< Number of turns before the effect expires.
        Unit* source = nullptr;  ///< The unit that applied this effect.
    };
}