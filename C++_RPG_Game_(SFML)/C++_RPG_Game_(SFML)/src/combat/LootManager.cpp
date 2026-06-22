#include "LootManager.h"
#include "AbilityFactory.h"
#include <cstdlib>

namespace RPG {

    int LootManager::rollGold(int minGold, int maxGold) {
        if (maxGold <= minGold) return minGold;
        return minGold + (rand() % ((maxGold - minGold) + 1));
    }

    // For battle victory - BattleManager is available
    std::vector<std::shared_ptr<IAbility>> LootManager::rollLoot(
        const std::vector<LootDrop>& drops, BattleManager& manager)
    {
        std::vector<std::shared_ptr<IAbility>> lootedItems;

        for (const auto& drop : drops) {
            float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            if (roll <= drop.chance) {
                auto item = AbilityFactory::getInstance().createAbility(drop.id, manager, nullptr);
                if (item) lootedItems.push_back(item);
            }
        }

        return lootedItems;
    }

    // For chest triggers - scans all loaded ability definitions,
    // rolls independently on each one that has chestChance > 0.
    // BattleManager is nullptr - assigned later by BattleManager::loadEncounter.
    std::vector<std::shared_ptr<IAbility>> LootManager::rollChestLoot()
    {
        std::vector<std::shared_ptr<IAbility>> lootedItems;

        const auto& definitions = AbilityFactory::getInstance().getDefinitions();

        for (const auto& [id, def] : definitions) {
            if (def.chestChance <= 0.f) continue;

            float roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            if (roll <= def.chestChance) {
                auto item = AbilityFactory::getInstance().createAbility(id);
                if (item) lootedItems.push_back(item);
            }
        }

        return lootedItems;
    }

}
