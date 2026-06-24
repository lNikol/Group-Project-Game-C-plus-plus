#include "UnitFactory.h"
#include "AbilityFactory.h"
#include "worldmap/AssetManager.h"
#include "combat/BattleManager.h"
#include <fstream>
#include <iostream>
#include <external/json.hpp> 

using json = nlohmann::json;

namespace RPG {

    UnitFactory& UnitFactory::getInstance() {
        static UnitFactory instance;
        return instance;
    }

    bool UnitFactory::loadFromJSON(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "CRITICAL: Failed to open unit file: " << filepath << std::endl;
            return false;
        }

        json j;
        try {
            file >> j;
        }
        catch (json::parse_error& e) {
            std::cerr << "JSON Parse Error: " << e.what() << std::endl;
            return false;
        }

        if (!j.contains("units") || !j["units"].is_array()) {
            std::cerr << "JSON Error: 'units' array missing in " << filepath << std::endl;
            return false;
        }

        for (const auto& item : j["units"]) {
            UnitDefinition def;

            def.id = item.value("id", "unknown");
            def.name = item.value("name", "Unnamed Unit");
            def.spritesheetId = item.value("spritesheetId", "BattleEnemies");

            // Texture Rect [x, y, w, h]
            if (item.contains("textureRect") && item["textureRect"].is_array() && item["textureRect"].size() == 4) {
                def.textureRect = sf::IntRect(
                    { item["textureRect"][0], item["textureRect"][1] },
                    { item["textureRect"][2], item["textureRect"][3] }
                );
            }
            else {
                def.textureRect = sf::IntRect({ 0, 0 }, { 32, 32 });
            }

            // Vitals
            float hp = item.value("hp", 100.f);
            float mp = item.value("mp", 50.f);
            float stamina = item.value("stamina", 50.f);
            def.baseVitals = { hp, hp, mp, mp, stamina, stamina };

            // Stats
            if (item.contains("stats")) {
                def.baseStats[StatType::Strength] = item["stats"].value("Strength", 10.f);
                def.baseStats[StatType::Dexterity] = item["stats"].value("Dexterity", 10.f);
                def.baseStats[StatType::Intellect] = item["stats"].value("Intellect", 10.f);
                def.baseStats[StatType::Vitality] = item["stats"].value("Vitality", 10.f);
            }

            // Combat Modifiers
            def.baseInitiative = item.value("initiative", 10);
            def.baseCritChance = item.value("critChance", 0.05f);
            def.baseDoubleTurnChance = item.value("doubleTurnChance", 0.0f);

            // Abilities Array
            if (item.contains("abilities") && item["abilities"].is_array()) {
                for (const auto& ab : item["abilities"]) {
                    def.defaultAbilities.push_back(ab.get<std::string>());
                }
            }

            m_definitions[def.id] = def;
            std::cout << "Loaded Unit Blueprint: " << def.name << " (" << def.id << ")" << std::endl;
        }

        return true;
    }

    std::shared_ptr<Unit> UnitFactory::createUnit(const std::string& id, Team team, BattleManager& manager, float logicalX, float logicalY) {
        if (m_definitions.find(id) == m_definitions.end()) {
            std::cerr << "Warning: Attempted to create unknown unit: " << id << std::endl;
            return nullptr;
        }

        const auto& def = m_definitions[id];

        // 1. Create the base unit
        auto unit = std::make_shared<Unit>(def.id, def.name, team, def.baseVitals);
        unit->setBaseStats(def.baseStats);
        unit->setInitiative(def.baseInitiative);
        unit->setCritChance(def.baseCritChance);
        unit->setDoubleTurnChance(def.baseDoubleTurnChance);

        // 2. Set spatial data
        unit->setLogicalPosition(logicalX, logicalY);

        // 3. Set visual sprite
        const auto* sheet = AssetManager::getInstance().getSpritesheet(def.spritesheetId);
        if (sheet) {
            unit->setSprite(*sheet, def.textureRect);
        }

        // 4. Bind Default Abilities via AbilityFactory
        int slotIndex = 0;
        for (const auto& abilityId : def.defaultAbilities) {
            auto ability = AbilityFactory::getInstance().createAbility(abilityId, manager, unit.get());
            if (ability) {
                unit->setHotbarAbility(slotIndex, ability);
                slotIndex++;
            }
        }

        return unit;
    }
}