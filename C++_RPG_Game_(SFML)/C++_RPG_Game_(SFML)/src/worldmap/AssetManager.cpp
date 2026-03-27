#include "AssetManager.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace RPG {

    // ==============================
    // Structure Management
    // ==============================

    void AssetManager::init() {

        structureLibrary.clear();
        prefixes.clear();

        // 1. Register the main atlas
        addTexture("world_atlas", GameConfig::WORLD_PATH + "world_atlas.png");

        // 2. Define reusable metadata templates
        StructureMetadata obstacle;
        obstacle.blocksMovement = true;
        obstacle.isDestructible = true;
        obstacle.maxHealth = 100.f;

        StructureMetadata floor;
        floor.blocksMovement = false;
        floor.speedModifier = 1.0f;

        // 3. Define Prefix Rules (Mapping file category to Game Logic)
        prefixes["tree"] = { StructureType::Tree, obstacle };
        prefixes["rock"] = { StructureType::Rock, obstacle };
        prefixes["grass"] = { StructureType::Grass, floor };
        prefixes["ground"] = { StructureType::Grass, floor };
        prefixes["house"] = { StructureType::House, obstacle };
        loadManifest(GameConfig::WORLD_PATH + "manifest.json", GameConfig::WORLD_PATH + "world_atlas");
    }

    void AssetManager::loadManifest(const std::string& path, const std::string& textureKey) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[AssetManager] ERROR: Could not open manifest: " << path << std::endl;
            return;
        }

        nlohmann::json manifest;
        file >> manifest;

        for (auto& [name, data] : manifest["definitions"].items()) {
            int id = data["id"];
            idToNameMap[id] = name;
            AssetTemplate* foundTemplate = nullptr;

            // Match the asset name (e.g., "trees_5") against our registered prefixes
            for (auto& [prefix, t] : prefixes) {
                if (name.find(prefix) != std::string::npos) {
                    foundTemplate = &t;
                    break;
                }
            }

            if (!foundTemplate) continue;

            // Construct the final StructureDefinition
            StructureDefinition def;
            def.name = name;
            def.type = foundTemplate->type;
            def.textureKey = textureKey;
            def.textureStartPos = { (uint32_t)data["x"], (uint32_t)data["y"] };
            def.size = { (uint32_t)data["w"], (uint32_t)data["h"] };

            // Precise data calculated by the Python processing script
            def.hitboxSize = { (float)data["hb_w"], (float)data["hb_h"] };
            def.hitboxOffset = { (float)data["off_x"], (float)data["off_y"] };

            def.meta = foundTemplate->meta;

            structureLibrary[name] = def;
        }
        std::cout << "[AssetManager] Successfully mapped " << structureLibrary.size() << " assets from manifest.\n";
    }

    std::string AssetManager::getNameById(const int& id) const {
        auto it = idToNameMap.find(id);
        return (it != idToNameMap.end()) ? it->second : "";
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
            //std::cout << "Loading spritesheet \"" + name + "\"\n";
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


    std::string AssetManager::getDefinitionByType(const StructureType& type) const {
        std::string def;
        switch (type) {
            // TODO : add others assets for ground
            case StructureType::Grass: def = "grass_0"; break;
            default: def = "grass_0"; break;
        }
        return def;
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
