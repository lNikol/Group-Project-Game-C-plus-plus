#include "AssetManager.h"
#include "core/GameConfig.h"
#include <filesystem>
#include <iostream>

void RPG::AssetManager::init(SpritesheetManager& sm) {
    /** * @brief Helper lambda to register structures and handle texture loading
     */
    auto addStruct = [&](StructureType type, std::string name, std::string fileName,
        StructureSize size, float speed, bool canSpawn,
        bool blocks, bool allowsRegen) {

            std::string key = "";
            if (!fileName.empty()) {
                // 1. Construct the full path using the global config
                std::string fullPath = GameConfig::TEXTURES_PATH + fileName;

                // 2. Register the file in SpritesheetManager
                sm.addSpritesheet(fullPath);

                // 3. Extract the key from filename (e.g., "blue_water.png" -> "blue_water")
                key = std::filesystem::path(fileName).stem().string();
            }

            // 4. Store the definition in the library
            structureLibrary[type] = { name, key, size, speed, canSpawn, blocks, allowsRegen };
        };

    structureLibrary.clear();

    // --- DATA REGISTRATION ---
    // Syntax: Type, Name, Filename, Size {W,H}, Speed, MobSpawn, Blocks, Regen

    // Default / Empty
    addStruct(StructureType::None, "Empty", "", { 1, 1 }, 1.0f, false, false, false);

    addStruct(StructureType::InvisibleBlock, "Reserved", "", { 1, 1 }, 0.0f, false, true, false);
    
    // Obstacles (0.0f speed = impassable if blocksMovement is true)
    addStruct(StructureType::Rock, "Rock", "rock.png", { 1, 1 }, 0.0f, false, true, false);
    addStruct(StructureType::Tree, "Tree", "tree.png", { 1, 1 }, 0.0f, false, true, false);
    addStruct(StructureType::Wall, "Wall", "wall.png", { 1, 1 }, 0.0f, false, true, false);

    // Normal Terrain
    addStruct(StructureType::Water, "Water", "blue_water.png", { 1, 1 }, 0.65f, false, false, false);
    addStruct(StructureType::River, "River", "river.png", { 1, 1 }, 0.85f, false, false, false);
    addStruct(StructureType::Swamp, "Swamp", "swamp.png", { 1, 1 }, 0.40f, false, false, false);
    addStruct(StructureType::Grass, "Grass", "grass.png", { 1, 1 }, 1.00f, true, false, false);
    addStruct(StructureType::Sand, "Sand", "sand.png", { 1, 1 }, 0.90f, true, false, false);

    // Special Zones
    addStruct(StructureType::SaveZone, "Save Zone", "saveZone.png", { 1, 1 }, 1.25f, false, false, true);
    addStruct(StructureType::Camp, "Camp", "camp.png", { 2, 2 }, 1.25f, false, true, true);
    addStruct(StructureType::FactionBase, "Faction Base", "factionBase.png", { 3, 3 }, 1.15f, false, true, true);

    std::cout << "AssetManager: Successfully initialized " << structureLibrary.size() << " structures.\n";
}

const RPG::StructureDefinition& RPG::AssetManager::getDefinition(StructureType type) const {
    auto it = structureLibrary.find(type);
    if (it != structureLibrary.end()) {
        return it->second;
    }
    // Safety fallback: return None if the type is not found
    return structureLibrary.at(StructureType::None);
}