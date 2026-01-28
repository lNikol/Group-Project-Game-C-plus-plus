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
        /*
        auto addStruct = [&](
            StructureType type, 
            const std::string& name, 
            const std::string& fileName,
            StructureSize size, 
            float speed, 
            bool canSpawn,
            bool blocks, 
            bool allowsRegen,
            uint8_t collisionHeight
        ) {

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
            structureLibrary[type] = { 
                name, 
                key, 
                size, 
                speed, 
                canSpawn, 
                blocks, 
                allowsRegen,
                collisionHeight
            };
        };
        */

        structureLibrary.clear();

        auto addStructDef = [&](
            const std::string& name,
            StructureType type,
            const std::string& textureKey,
            sf::Vector2u textureStartPos,
            sf::Vector2u size,
            sf::Vector2f hitboxSize,
            sf::Vector2f hitboxOffset,
            StructureMetadata metadata
        ) {
            if (!textureKey.empty()) {
                std::string fullPath = GameConfig::TEXTURES_PATH + textureKey;
                addSpritesheet(name, fullPath);
            }

            structureLibrary[name] = {
                name,
                type,
                textureKey,
                textureStartPos,
                size,
                hitboxSize,
                hitboxOffset,
                metadata
            };
        };

        StructureMetadata meta;
        meta.blocksMovement = true;

        addStructDef(
            "rock",
            StructureType::Rock,
            "rock.png",
            { 0, 0 },
            { 32, 32 },
            { 20.f, 16.f },
            { 6.f, 16.f },
            meta
        );

        addStructDef(
            "big_tree",
            StructureType::Tree,
            "trees.png",
            { 16, 0 },
            { 128, 160 },
            { 16.f, 16.f },
            { 56.f, 140.f },
            meta
        );

        addStructDef(
            "house",
            StructureType::Camp,
            "house.png",
            { 0, 0 },
            { 96, 128 },
            { 72.f, 48.f },
            { 12.f, 70.f },
            meta
        );

        /*

        // --- DATA REGISTRATION ---
        // Syntax: Type, Name, Filename, Size {W,H}, Speed, CanSpawn, Blocks, AllowsRegen

        addStruct(StructureType::None, "Empty", "", { 1,1 }, 1.0f, false, false, false, 0);
        addStruct(StructureType::InvisibleBlock, "Reserved", "", { 1,1 }, 0.0f, false, true, false, GameConfig::TILE_SIZE);

        // Obstacles
        addStruct(StructureType::Rock, "Rock", "rock.png", { 1,1 }, 0.0f, false, true, false, 12);
        addStruct(StructureType::Tree, "Tree", "tree.png", { 1,1 }, 0.0f, false, true, false, 12);
        addStruct(StructureType::Wall, "Wall", "wall.png", { 1,1 }, 0.0f, false, true, false, GameConfig::TILE_SIZE);

        // Normal Terrain
        addStruct(StructureType::Water, "Water", "blue_water.png", { 1,1 }, 0.65f, false, false, false, 0);
        addStruct(StructureType::River, "River", "river.png", { 1,1 }, 0.85f, false, false, false, 0);
        addStruct(StructureType::Swamp, "Swamp", "swamp.png", { 1,1 }, 0.40f, false, false, false, 0);
        addStruct(StructureType::Grass, "Grass", "grass.png", { 1,1 }, 1.0f, true, false, false, 0);
        addStruct(StructureType::Sand, "Sand", "sand.png", { 1,1 }, 0.90f, true, false, false, 0);

        // Special Zones
        addStruct(StructureType::SaveZone, "Save Zone", "saveZone.png", { 1,1 }, 1.25f, false, false, true, 0);
        addStruct(StructureType::Camp, "Camp", "camp.png", { 2,2 }, 1.25f, false, true, true, 0);
        addStruct(StructureType::FactionBase, "Faction Base", "factionBase.png", { 3,3 }, 1.15f, false, true, true, 0);

        */
        std::cout << "AssetManager: Successfully initialized "
            << structureLibrary.size() << " structures.\n";
    }

    const StructureDefinition& AssetManager::getDefinition(const std::string& name) const {
        auto it = structureLibrary.find(name);
        return (it != structureLibrary.end()) ? it->second : structureLibrary.at(name);
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

        std::cout << "Spritesheet \"" + name + "\" loaded from: \"" + filepath + "\".\n";
        spritesheetMap.emplace(name, std::move(spritesheet));
    }

    const Spritesheet* AssetManager::getSpritesheet(const std::string& name) const {
        auto it = spritesheetMap.find(name);
        
        if (it != spritesheetMap.end()) {
            std::cout << "Loading spritesheet \"" + name + "\"\n";
            return &it->second;
        }

        std::cout << "Failed to load spritesheet \"" + name + "\"\n";
        return nullptr;
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
