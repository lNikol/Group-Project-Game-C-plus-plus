#pragma once
#include "combat/Unit.h"
#include "combat/AbilityData.h" // For UnitStats
#include "interfaces/ICombatActor.h" // For Vitals
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <SFML/Graphics/Rect.hpp>

namespace RPG {

    class BattleManager;

    struct UnitDefinition {
        std::string id;
        std::string name;
        std::string spritesheetId;
        sf::IntRect textureRect;

        Vitals baseVitals;
        UnitStats baseStats;

        uint8_t baseInitiative = 10;
        float baseCritChance = 0.0f;
        float baseDoubleTurnChance = 0.0f;

        std::vector<std::string> defaultAbilities;
    };

    /**
     * @brief Singleton Factory managing UnitDefinitions and instantiating Units.
     */
    class UnitFactory {
    public:
        static UnitFactory& getInstance();

        /**
         * @brief Loads and parses unit blueprints from a JSON file.
         */
        bool loadFromJSON(const std::string& filepath);
      
        /**
         * @brief Creates a fully initialized Unit instance.
         * @param id The string key (e.g., "dark_mage").
         * @param team The team alignment (Player, Enemy, Neutral).
         * @param manager Reference to BattleManager (needed for Ability creation).
         * @param logicalX Starting X position.
         * @param logicalY Starting Y position.
         * @return std::shared_ptr<Unit> or nullptr if ID not found.
         */
        std::shared_ptr<Unit> createUnit(const std::string& id, Team team, BattleManager& manager, float logicalX, float logicalY);

    private:
        UnitFactory() = default;
        std::unordered_map<std::string, UnitDefinition> m_definitions;
    };
}