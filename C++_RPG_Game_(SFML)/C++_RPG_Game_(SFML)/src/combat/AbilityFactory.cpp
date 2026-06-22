#include "AbilityFactory.h"
#include "combat/CombatAbility.h" 
#include "combat/CombatItem.h"
#include <fstream>
#include <iostream>
#include <external/json.hpp> 

using json = nlohmann::json;

namespace RPG {

    AbilityFactory& AbilityFactory::getInstance() {
        static AbilityFactory instance;
        return instance;
    }

    // ==========================================
    // String to Enum Converters
    // ==========================================

    StatType AbilityFactory::parseStatType(const std::string& str) const {
        if (str == "Strength") return StatType::Strength;
        if (str == "Dexterity") return StatType::Dexterity;
        if (str == "Intellect") return StatType::Intellect;
        if (str == "Vitality") return StatType::Vitality;
        return StatType::None;
    }

    EffectType AbilityFactory::parseEffectType(const std::string& str) const {
        if (str == "Heal") return EffectType::Heal;
        if (str == "Buff") return EffectType::Buff;
        if (str == "Debuff") return EffectType::Debuff;
        if (str == "Stun") return EffectType::Stun;
        return EffectType::Damage; // Default
    }

    TargetType AbilityFactory::parseTargetType(const std::string& str) const {
        if (str == "Self") return TargetType::Self;
        if (str == "SingleAlly") return TargetType::SingleAlly;
        if (str == "AreaEnemy") return TargetType::AreaEnemy;
        if (str == "AreaAlly") return TargetType::AreaAlly;
        return TargetType::SingleEnemy; // Default
    }

    // ==========================================
    // JSON Loading Logic
    // ==========================================

    bool AbilityFactory::loadFromJSON(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "CRITICAL: Failed to open ability file: " << filepath << std::endl;
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

        if (!j.contains("abilities") || !j["abilities"].is_array()) {
            std::cerr << "JSON Error: 'abilities' array missing." << std::endl;
            return false;
        }

        for (const auto& item : j["abilities"]) {
            AbilityDefinition def;

            def.id = item.value("id", "unknown");
            def.name = item.value("name", "Unnamed Ability");
            def.filePath = item.value("filePath", "");
            def.description = item.value("description", "");
            def.iconIndex = item.value("iconIndex", 0);
            def.targetType = parseTargetType(item.value("targetType", "SingleEnemy"));
            def.cooldownTurns = item.value("cooldownTurns", 0);
            def.range = item.value("range", 100.f);
            def.radius = item.value("radius", 0.f);
            def.manaCost = item.value("manaCost", 0.f);
            def.staminaCost = item.value("staminaCost", 0.f);
            def.chestChance = item.value("chestChance", 0.0f);
            def.isConsumable = item.value("isConsumable", false);
            def.maxCharges = item.value("maxCharges", -1);
            if (item.contains("effects") && item["effects"].is_array()) {
                for (const auto& eff : item["effects"]) {
                    EffectData effectData;

                    effectData.type = parseEffectType(eff.value("type", "Damage"));
                    effectData.baseValue = eff.value("baseValue", 0.f);
                    effectData.scalingStat = parseStatType(eff.value("scalingStat", "None"));
                    effectData.scalingFactor = eff.value("scalingFactor", 0.f);
                    effectData.targetStat = parseStatType(eff.value("targetStat", "None"));

                    effectData.baseCritChance = eff.value("baseCritChance", 0.0f);
                    effectData.critScalingStat = parseStatType(eff.value("critScalingStat", "None"));
                    effectData.critScalingFactor = eff.value("critScalingFactor", 0.f);
                    effectData.critMultiplier = eff.value("critMultiplier", 1.5f);

                    effectData.durationTurns = eff.value("durationTurns", 0);
                    effectData.applyChance = eff.value("applyChance", 1.0f);

                    def.effects.push_back(effectData);
                }
            }

            m_definitions[def.id] = def;
            std::cout << "Loaded Ability: " << def.name << " (" << def.id << ")" << std::endl;
        }

        return true;
    }

    std::shared_ptr<CombatAbility> AbilityFactory::createAbility(const std::string& id, BattleManager& manager, Unit* owner) {
        if (m_definitions.find(id) == m_definitions.end()) {
            std::cerr << "Warning: Attempted to create unknown ability: " << id << std::endl;
            return nullptr;
        }
        auto& def = m_definitions[id];
        if (def.isConsumable) {
            return std::make_shared<CombatItem>(&def, &manager, owner);
        }
        return std::make_shared<CombatAbility>(&def, &manager, owner);
    }

    std::shared_ptr<CombatAbility> AbilityFactory::createAbility(const std::string& id) {
        if (m_definitions.find(id) == m_definitions.end()) {
            std::cerr << "Warning*: Attempted to create unknown ability: " << id << std::endl;
            return nullptr;
        }
        auto& def = m_definitions[id];
        if (def.isConsumable) {
            return std::make_shared<CombatItem>(&def, nullptr, nullptr);
        }
        return std::make_shared<CombatAbility>(&def, nullptr, nullptr);
    }

    const std::unordered_map<std::string, AbilityDefinition>& AbilityFactory::getDefinitions() const {
        return m_definitions;
    }
}