#include "AssetManager.h"

namespace RPG {

    // ==============================
    // Structure Management
    // ==============================

    void AssetManager::init() {
        /**
         * @brief Helper lambda to register structure definitions
         *        and handle texture registration.
         */
        auto addStruct = [&](StructureType type, const std::string& name, const std::string& fileName,
            StructureSize size, float speed, bool canSpawn,
            bool blocks, bool allowsRegen) {

                std::string key;
                if (!fileName.empty()) {
                    // Construct full path using global config
                    std::string fullPath = GameConfig::TEXTURES_PATH + fileName;

                    // Extract key from filename (e.g., "blue_water.png" -> "blue_water")
                    key = std::filesystem::path(fileName).stem().string();

                    // Register spritesheet
                    addSpritesheet(key, fullPath);
                }

                // Store definition in library
                structureLibrary[type] = { name, key, size, speed, canSpawn, blocks, allowsRegen };
            };

        structureLibrary.clear();

        // --- DATA REGISTRATION ---
        // Syntax: Type, Name, Filename, Size {W,H}, Speed, CanSpawn, Blocks, AllowsRegen

        addStruct(StructureType::None, "Empty", "", { 1,1 }, 1.0f, false, false, false);
        addStruct(StructureType::InvisibleBlock, "Reserved", "", { 1,1 }, 0.0f, false, true, false);

        // Obstacles
        addStruct(StructureType::Rock, "Rock", "rock.png", { 1,1 }, 0.0f, false, true, false);
        addStruct(StructureType::Tree, "Tree", "tree.png", { 1,1 }, 0.0f, false, true, false);
        addStruct(StructureType::Wall, "Wall", "wall.png", { 1,1 }, 0.0f, false, true, false);

        // Normal Terrain
        addStruct(StructureType::Water, "Water", "blue_water.png", { 1,1 }, 0.65f, false, false, false);
        addStruct(StructureType::River, "River", "river.png", { 1,1 }, 0.85f, false, false, false);
        addStruct(StructureType::Swamp, "Swamp", "swamp.png", { 1,1 }, 0.40f, false, false, false);
        addStruct(StructureType::Grass, "Grass", "grass.png", { 1,1 }, 1.0f, true, false, false);
        addStruct(StructureType::Sand, "Sand", "sand.png", { 1,1 }, 0.90f, true, false, false);

        // Special Zones
        addStruct(StructureType::SaveZone, "Save Zone", "saveZone.png", { 1,1 }, 1.25f, false, false, true);
        addStruct(StructureType::Camp, "Camp", "camp.png", { 2,2 }, 1.25f, false, true, true);
        addStruct(StructureType::FactionBase, "Faction Base", "factionBase.png", { 3,3 }, 1.15f, false, true, true);

        std::cout << "AssetManager: Successfully initialized "
            << structureLibrary.size() << " structures.\n";
    }

    const StructureDefinition& AssetManager::getDefinition(StructureType type) const {
        auto it = structureLibrary.find(type);
        return (it != structureLibrary.end()) ? it->second : structureLibrary.at(StructureType::None);
    }

    // ==============================
    // Spritesheets
    // ==============================

    void AssetManager::addSpritesheet(const std::string& name, const std::string& filepath) {
        if (spritesheetMap.count(name) > 0) {
            std::cerr << "[AssetManager] Spritesheet \"" << name << "\" already exists!\n";
            return;
        }

        Spritesheet spritesheet;
        if (!spritesheet.loadFromFile(filepath)) {
            std::cerr << "[AssetManager] Failed to load Spritesheet: \"" << filepath << "\"\n";
            return;
        }

        spritesheetMap.emplace(name, std::move(spritesheet));
    }

    const Spritesheet* AssetManager::getSpritesheet(const std::string& name) const {
        auto it = spritesheetMap.find(name);
        return (it != spritesheetMap.end()) ? &it->second : nullptr;
    }

    // ==============================
    // Textures
    // ==============================

    void AssetManager::addTexture(const std::string& name, const std::string& filepath) {
        if (textureMap.count(name) > 0) {
            std::cerr << "[AssetManager] Texture \"" << name << "\" already exists!\n";
            return;
        }

        sf::Texture texture;
        if (!texture.loadFromFile(filepath)) {
            std::cerr << "[AssetManager] Failed to load Texture: \"" << filepath << "\"\n";
            return;
        }

        textureMap.emplace(name, std::move(texture));
    }

    const sf::Texture* AssetManager::getTexture(const std::string& name) const {
        auto it = textureMap.find(name);
        return (it != textureMap.end()) ? &it->second : nullptr;
    }

    // ==============================
    // Fonts
    // ==============================

    void AssetManager::addFont(const std::string& name, const std::string& filepath) {
        if (fontMap.count(name) > 0) {
            std::cerr << "[AssetManager] Font \"" << name << "\" already exists!\n";
            return;
        }

        sf::Font font;
        if (!font.openFromFile(filepath)) {
            std::cerr << "[AssetManager] Failed to load Font: \"" << filepath << "\"\n";
            return;
        }

        fontMap.emplace(name, std::move(font));
    }

    const sf::Font* AssetManager::getFont(const std::string& name) const {
        auto it = fontMap.find(name);
        return (it != fontMap.end()) ? &it->second : nullptr;
    }

}
