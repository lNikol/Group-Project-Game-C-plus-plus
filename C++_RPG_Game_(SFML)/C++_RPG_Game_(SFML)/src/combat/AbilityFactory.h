#pragma once
#include "AbilityData.h"
#include <unordered_map>
#include <memory>
#include <string>

namespace RPG {

    class CombatAbility;
    class BattleManager;
    class Unit;

    /**
     * @brief Singleton Factory managing AbilityDefinitions and creating runtime instances.
     * Supports loading definitions directly from external JSON files.
     */
    class AbilityFactory {
    public:
        static AbilityFactory& getInstance();

        /**
         * @brief Loads and parses ability blueprints from a JSON file.
         * @param filepath The path to the abilities.json file.
         * @return True if successful, false if file not found or parsing failed.
         */
        bool loadFromJSON(const std::string& filepath);

        /**
         * @brief Creates a usable ability instance from a definition ID.
         * @param id The string key (e.g., "slash").
         * @param manager Reference to BattleManager.
         * @param owner The Unit that owns this skill.
         * @return std::shared_ptr<CombatAbility> or nullptr if ID not found.
         */
        std::shared_ptr<CombatAbility> createAbility(const std::string& id, BattleManager& manager, Unit* owner);

        /**
         * @brief Creates an ability without a BattleManager (for chest loot).
         * BattleManager will be assigned by BattleManager::loadEncounter
         * when the item is found in PartyData::sharedInventory.
         */
        std::shared_ptr<CombatAbility> createAbility(const std::string& id);


        const std::unordered_map<std::string, AbilityDefinition>& getDefinitions() const;


    private:
        AbilityFactory() = default;
        std::unordered_map<std::string, AbilityDefinition> m_definitions;

        // --- Enum Parsers ---
        StatType parseStatType(const std::string& str) const;
        EffectType parseEffectType(const std::string& str) const;
        TargetType parseTargetType(const std::string& str) const;
        EquipSlot parseEquipSlot(const std::string& str) const;
    };
}