#include "worldmap/MapLoader.h"
#include "worldmap/AssetManager.h"
#include "game_objects/GameObjectFactory.h"
#include "game_objects/components/ColliderComponent.h"
#include "core/GameConfig.h"
#include <fstream>
#include <iostream>
#include <cmath>

namespace RPG {

    int32_t MapLoader::offsetX = 0;
    int32_t MapLoader::offsetY = 0;

    bool MapLoader::loadFromTiled(const std::string& path, WorldMap& map) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[MapLoader] ERROR: Cannot open: " << path << "\n";
            return false;
        }

        nlohmann::json tmj;
        file >> tmj;

        int32_t minTileX = INT32_MAX, maxTileX = INT32_MIN;
        int32_t minTileY = INT32_MAX, maxTileY = INT32_MIN;
        bool isInfinite = tmj.value("infinite", false);

        if (isInfinite) {
            for (const auto& layer : tmj["layers"]) {
                if (layer["type"] == "tilelayer" && layer.contains("chunks")) {
                    for (const auto& chunk : layer["chunks"]) {
                        int32_t cx = chunk["x"], cy = chunk["y"];
                        int32_t cw = chunk["width"], ch = chunk["height"];
                        minTileX = std::min(minTileX, cx); maxTileX = std::max(maxTileX, cx + cw);
                        minTileY = std::min(minTileY, cy); maxTileY = std::max(maxTileY, cy + ch);
                    }
                }
            }
        }

        if (minTileX == INT32_MAX) {
            minTileX = 0; minTileY = 0;
            maxTileX = tmj.value("width", 0);
            maxTileY = tmj.value("height", 0);
        }

        offsetX = minTileX;
        offsetY = minTileY;
        map.reshape(maxTileX - minTileX, maxTileY - minTileY, 0, 0);

        for (auto& layer : tmj["layers"]) {
            if      (layer["type"] == "tilelayer")   processTileLayer(layer, map);
            else if (layer["type"] == "objectgroup") processObjectLayer(layer, map);
        }
        return true;
    }

    bool MapLoader::placeStructure(WorldMap& map, float worldX, float worldY,
                                   const std::string& id, uint8_t zIndex) {
        AssetManager& am = AssetManager::getInstance();
        if (!am.hasDefinition(id)) return false;

        const StructureDefinition& def = am.getDefinition(id);

        auto obj = Factory::createDynamicObject(am, id, { worldX, worldY });
        obj->setZIndex(zIndex);
        obj->setPosition({ worldX, worldY });

        if (auto* col = obj->getComponent<ColliderComponent>()) {
            sf::FloatRect hb = col->getGlobalHitbox();

            int32_t sx = static_cast<int32_t>(std::floor(hb.position.x / GameConfig::TILE_SIZE));
            int32_t sy = static_cast<int32_t>(std::floor(hb.position.y / GameConfig::TILE_SIZE));
            int32_t ex = static_cast<int32_t>(std::floor((hb.position.x + hb.size.x - 0.1f) / GameConfig::TILE_SIZE));
            int32_t ey = static_cast<int32_t>(std::floor((hb.position.y + hb.size.y - 0.1f) / GameConfig::TILE_SIZE));

            if (sx < 0 || sy < 0 ||
                ex >= static_cast<int32_t>(map.getWidth()) ||
                ey >= static_cast<int32_t>(map.getHeight())) {
                std::cerr << "[MapLoader] Rejected '" << id << "' — out of bounds.\n";
                return false;
            }

            for (int32_t y = sy; y <= ey; ++y)
                for (int32_t x = sx; x <= ex; ++x)
                    if (!map.at(x, y).canAccept(def.layer)) return false;

            for (int32_t y = sy; y <= ey; ++y)
                for (int32_t x = sx; x <= ex; ++x)
                    map.at(x, y).blocksPlacement |= def.meta.blocksPlacement;
        }
        else {
            // Structures witghout ColliderComponent (decorations) can set blocksMovement
            // through the tilemap because they don't have a precise hitbox
        }

        map.addGameObject(std::move(obj));
        return true;
    }

    void MapLoader::processTileLayer(const nlohmann::json& layer, WorldMap& map) {
        if (!layer.contains("chunks")) return;
        for (const auto& chunk : layer["chunks"]) {
            int32_t  cx = chunk["x"], cy = chunk["y"];
            uint32_t cw = chunk["width"];
            const auto& data = chunk["data"];
            for (uint32_t i = 0; i < data.size(); ++i) {
                uint32_t gid = data[i];
                if (gid == 0) continue;
                int32_t lx = (cx + static_cast<int32_t>(i % cw)) - offsetX;
                int32_t ly = (cy + static_cast<int32_t>(i / cw)) - offsetY;
                if (lx >= 0 && lx < static_cast<int32_t>(map.getWidth()) &&
                    ly >= 0 && ly < static_cast<int32_t>(map.getHeight()))
                    map.assignGroundType(lx, ly, gid);
            }
        }
    }

    void MapLoader::processObjectLayer(const nlohmann::json& layer, WorldMap& map) {
        uint8_t zIndex = determineZIndex(layer);
        AssetManager& am = AssetManager::getInstance();

        for (const auto& obj : layer["objects"]) {
            std::string type = obj.value("type", "");
            int interactId  = getIntProperty(obj, "interaction_id", -1);
            std::string objName = obj.value("name", "");

            if (obj.contains("gid")) {
                uint32_t gid = obj["gid"];
                std::string assetName = am.getNameById(gid - 1);
                if (assetName.empty()) continue;

                const StructureDefinition& def = am.getDefinition(assetName);
                float wx = (static_cast<float>(obj["x"]) + def.size.x / 2.f)
                           - offsetX * GameConfig::TILE_SIZE;
                float wy = static_cast<float>(obj["y"])
                           - offsetY * GameConfig::TILE_SIZE;

                placeStructure(map, wx, wy, assetName, zIndex);
            }
            else {
                float rx = static_cast<float>(obj["x"]) - offsetX * GameConfig::TILE_SIZE;
                float ry = static_cast<float>(obj["y"]) - offsetY * GameConfig::TILE_SIZE;

                if (type == "player_spawn") {
                    map.setPlayerInitialPosition(rx, ry);
                }
                else if (type == "npc_spawn") {
                    FactionID faction = mapFaction(getStrProperty(obj, "faction", "neutral"));
                    std::string texture = getStrProperty(obj, "texture", "npc");
                    int dialogId = getIntProperty(obj, "dialog_id", -1);
                    bool isLeader = getBoolProperty(obj, "is_leader", false);
                    
                    map.spawnNPC(rx, ry, faction, texture, dialogId, isLeader);
                }
                else if (type == "trigger") {
                    float rw = obj.value("width", 64.f);
                    float rh = obj.value("height", 64.f);

                    auto triggerObj = std::make_unique<GameObject>("trigger");
                    triggerObj->setPosition({ rx, ry });

                    auto* tc = triggerObj->addComponent<TriggerComponent>();

                    bool isEllipse = obj.value("ellipse", false);
                    if (isEllipse) {
                        tc->shape = TriggerShape::Circle;
                        tc->radius = rw / 2.f; 
                    }
                    else {
                        tc->shape = TriggerShape::Rect;
                        tc->rectSize = { rw, rh };
                    }

                    tc->action = mapTrigger(getStrProperty(obj, "action", ""));
                    tc->targetScene = mapFaction(getStrProperty(obj, "targetScene", "neutral"));
                    tc->dialogId = getIntProperty(obj, "dialogId", -1);
                    tc->chestId =  getIntProperty(obj, "chestId", -1);
                    tc->oneShot =  getStrProperty(obj, "oneShot", "true") == "true";

                    map.addGameObject(std::move(triggerObj));
                }
            }
        }
    }

    FactionID MapLoader::mapFaction(std::string name) {
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        static const std::unordered_map<std::string, FactionID> m = {
            {"main",    FactionID::MainWorld},
            {"white",   FactionID::WhiteOrder},
            {"dark",    FactionID::DarkOrder},
            {"neutral", FactionID::NeutralOrder},
            {"battle",  FactionID::BattleScene}
        };
        auto it = m.find(name);
        return (it != m.end()) ? it->second : FactionID::NeutralOrder;
    }


    TriggerAction MapLoader::mapTrigger(std::string name) {
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        static const std::unordered_map<std::string, TriggerAction> m = {
            {"changescene", TriggerAction::ChangeScene},
            {"openchest",   TriggerAction::OpenChest},
            {"startdialog", TriggerAction::StartDialog},
            {"savegame",    TriggerAction::SaveGame}
        };
        auto it = m.find(name);
        return (it != m.end()) ? it->second : TriggerAction::ChangeScene;
    }

    uint8_t MapLoader::determineZIndex(const nlohmann::json& layer) {
        std::string z = getStrProperty(layer, "zIndex", "main");
        std::transform(z.begin(), z.end(), z.begin(), ::tolower);
        static const std::unordered_map<std::string, uint8_t> zm = {
            {"ground",      (uint8_t)LayerZIndex::ZLayer::Ground},
            {"belowplayer", (uint8_t)LayerZIndex::ZLayer::BelowPlayer},
            {"main",        (uint8_t)LayerZIndex::ZLayer::Main},
            {"aboveplayer", (uint8_t)LayerZIndex::ZLayer::AbovePlayer},
            {"ui",          (uint8_t)LayerZIndex::ZLayer::UI}
        };
        auto it = zm.find(z);
        return (it != zm.end()) ? it->second : (uint8_t)LayerZIndex::ZLayer::Main;
    }

    std::string MapLoader::getStrProperty(const nlohmann::json& el, const std::string& name, const std::string& def) {
        if (el.contains("properties"))
            for (const auto& p : el["properties"])
                if (p["name"] == name && p["value"].is_string())
                    return p["value"].get<std::string>();
        return def;
    }

    int MapLoader::getIntProperty(const nlohmann::json& el, const std::string& name, int def) {
        if (el.contains("properties"))
            for (const auto& p : el["properties"])
                if (p["name"] == name && p["value"].is_number())
                    return p["value"].get<int>();
        return def;
    }

    bool MapLoader::getBoolProperty(const nlohmann::json& el, const std::string& name, bool def) {
        if (el.contains("properties"))
            for (const auto& p : el["properties"])
                if (p["name"] == name && p["value"].is_boolean())
                    return p["value"].get<bool>();
        return def;
    }
}