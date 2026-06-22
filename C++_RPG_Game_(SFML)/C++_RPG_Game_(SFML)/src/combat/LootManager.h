#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Interfaces/IAbility.h"
#include "EncounterLoader.h"

namespace RPG {

    class BattleManager;

    class LootManager {
    public:
        /**
         * @brief Rolls gold reward in range [minGold, maxGold].
         */
        static int rollGold(int minGold, int maxGold);

        /**
         * @brief Rolls loot for a battle victory.
         * Abilities are created with a valid BattleManager reference.
         */
        static std::vector<std::shared_ptr<IAbility>> rollLoot(
            const std::vector<LootDrop>& drops, BattleManager& manager);

        /**
         * @brief Rolls loot for a chest trigger.
         * Iterates ALL ability definitions loaded in AbilityFactory,
         * filters those with chestChance > 0, and rolls each independently.
         * BattleManager is nullptr - assigned later by BattleManager::loadEncounter.
         */
        static std::vector<std::shared_ptr<IAbility>> rollChestLoot();
    };

}
