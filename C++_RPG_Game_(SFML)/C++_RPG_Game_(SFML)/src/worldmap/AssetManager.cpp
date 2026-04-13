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
        idToNameMap.clear();

        // 1. Register the main atlas
        addTexture("world_atlas", GameConfig::WORLD_PATH + "world_atlas.png");

        loadTemplates(GameConfig::JSON_PATH + "templates.json");

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
            int32_t id = data["id"];
            idToNameMap[id] = name;
            AssetTemplate* foundTemplate = nullptr;

            // Match the asset name (e.g., "trees_5") against our registered prefixes
            for (auto& [prefix, t] : prefixes) {
                if (name.compare(0, prefix.length(), prefix) == 0) {
                    foundTemplate = &t;
                    break;
                }
            }

            if (!foundTemplate) {
                std::cerr << "[AssetManager] Warning: No prefix match for asset: " << name
                    << ". Skipping from structure library." << std::endl;
                continue;
            }

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
            def.layer = foundTemplate->layer;

            structureLibrary[name] = def;
        }
        std::cout << "[AssetManager] Successfully mapped " << structureLibrary.size() << " assets from manifest.\n";
    }

    void AssetManager::loadTemplates(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[AssetManager] ERROR: Could not open templates file: " << path << std::endl;
            return;
        }

        nlohmann::json j;
        try {
            file >> j;
        }
        catch (const nlohmann::json::parse_error& e) {
            std::cerr << "[AssetManager] JSON Parse Error: " << e.what() << std::endl;
            return;
        }

        for (auto& item : j["templates"]) {
            AssetTemplate t;
            t.type = stringToType(item.value("type", "None"));
            t.layer = stringToLayer(item.value("layer", "Decoration"));

            auto& m = item["meta"];
            t.meta.blocksMovement = m.value("blocksMovement", false);
            t.meta.blocksPlacement = m.value("blocksPlacement", false);
            t.meta.isDestructible = m.value("isDestructible", false);
            t.meta.maxHealth = m.value("maxHealth", 100.0f);
            t.meta.speedModifier = m.value("speedModifier", 1.0f);

            if (item.contains("prefixes") && item["prefixes"].is_array()) {
                for (const std::string& pref : item["prefixes"]) {
                    prefixes[pref] = t;
                }
            }
            else if (item.contains("prefix")) {
                std::string pref = item["prefix"];
                prefixes[pref] = t;
            }
        }

        std::cout << "[AssetManager] Data-driven templates loaded successfully. Total prefixes: " << prefixes.size() << "\n";
    }

    PlacementLayer AssetManager::stringToLayer(const std::string& str) {
        static const std::unordered_map<std::string, PlacementLayer> mapper = {
            {"Ground",    PlacementLayer::Ground},
            {"Decoration",    PlacementLayer::Decoration},
            {"Object",    PlacementLayer::Object},
            {"Structure", PlacementLayer::Structure}
        };

        auto it = mapper.find(str);
        if (it != mapper.end()) return it->second;

        std::cerr << "[AssetManager] WARNING: Unknown layer '" << str << "'. Defaulting to Decoration.\n";
        return PlacementLayer::Decoration;
    }

    StructureType AssetManager::stringToType(const std::string& str) {
        static const std::unordered_map<std::string, StructureType> mapper = {
            {"None",  StructureType::None},
            {"Tree",  StructureType::Tree},
            {"Rock",  StructureType::Rock},
            {"Grass", StructureType::Grass},
            {"Wall",  StructureType::Wall},
            {"House", StructureType::House}
        };

        auto it = mapper.find(str);
        if (it != mapper.end()) return it->second;

        return StructureType::None;
    }

    std::string AssetManager::getNameById(const uint32_t& id) const {
        auto it = idToNameMap.find(id);
        return (it != idToNameMap.end()) ? it->second : "";
    }

    const StructureDefinition& AssetManager::getDefinition(const std::string& name) const {
        auto it = structureLibrary.find(name);
        return (it != structureLibrary.end()) ? it->second : structureLibrary.at(name);
    }

    StructureDefinition& AssetManager::getDefinition(const std::string& name) {
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

        return nullptr;
    }

    const Spritesheet& AssetManager::getTest(const std::string& name) const {
        auto it = spritesheetMap.find(name);

        if (it != spritesheetMap.end()) {
            std::cout << "Loading spritesheet \"" + name + "\"\n";
            return it->second;
        }

        std::cout << "Failed to load spritesheet \"" + name + "\"\n";
        return Spritesheet();
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
        for (auto const& [name, def] : structureLibrary) {
            if (def.type == type) return name;
        }
        std::cout << "Out of getdefbytype" << std::endl;
        return "grass_0"; // default
    }

    // ==============================d
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
