#include "PropFactory.h"
#include "worldmap/AssetManager.h" 
#include <fstream>
#include <iostream>
#include <external/json.hpp> 

using json = nlohmann::json;

namespace RPG {

    PropFactory& PropFactory::getInstance() {
        static PropFactory instance;
        return instance;
    }

    bool PropFactory::loadFromJSON(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "CRITICAL: Failed to open prop file: " << filepath << std::endl;
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

        if (!j.contains("props") || !j["props"].is_array()) {
            std::cerr << "JSON Error: 'props' array missing in " << filepath << std::endl;
            return false;
        }

        for (const auto& item : j["props"]) {
            PropDefinition def;

            def.id = item.value("id", "unknown");
            def.name = item.value("name", "Unnamed Prop");
            def.spritesheetId = item.value("spritesheetId", "BattleProps");

            // Parse IntRect as an array: [x, y, width, height]
            if (item.contains("textureRect") && item["textureRect"].is_array() && item["textureRect"].size() == 4) {
                // SFML 3 requires sf::IntRect({x, y}, {width, height})
                def.textureRect = sf::IntRect(
                    { item["textureRect"][0], item["textureRect"][1] },
                    { item["textureRect"][2], item["textureRect"][3] }
                );
            }
            else {
                def.textureRect = sf::IntRect({ 0, 0 }, { 32, 32 }); // Fallback
            }

            def.colliderWidth = item.value("colliderWidth", 30.f);
            def.colliderHeight = item.value("colliderHeight", 30.f);
            def.blocksMovement = item.value("blocksMovement", true);
            def.blocksSight = item.value("blocksSight", false);

            m_definitions[def.id] = def;
            std::cout << "Loaded Prop Blueprint: " << def.name << " (" << def.id << ")" << std::endl;
        }

        return true;
    }

    std::shared_ptr<Prop> PropFactory::createProp(const std::string& id, float logicalX, float logicalY) {
        if (m_definitions.find(id) == m_definitions.end()) {
            std::cerr << "Warning: Attempted to create unknown prop: " << id << std::endl;
            return nullptr;
        }

        const auto& def = m_definitions[id];

        // 1. Instantiate using the constructor defined in Prop.h
        auto prop = std::make_shared<Prop>(def.blocksMovement, def.blocksSight, def.name);

        // 2. Set physics data using CombatWorldObject methods
        prop->setColliderSize(def.colliderWidth, def.colliderHeight);
        prop->setLogicalPosition(logicalX, logicalY);

        // 3. Hook up the visual sprite
        const auto* sheet = AssetManager::getInstance().getSpritesheet(def.spritesheetId);
        if (sheet) {
            prop->setSprite(*sheet, def.textureRect);
        }
        else {
            std::cerr << "Warning: Prop " << id << " could not find spritesheet " << def.spritesheetId << std::endl;
        }

        return prop;
    }
}