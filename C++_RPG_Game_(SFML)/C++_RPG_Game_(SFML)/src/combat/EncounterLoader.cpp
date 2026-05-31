#include "EncounterLoader.h"
#include <external/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace RPG {

    std::optional<EncounterData> EncounterLoader::loadFromFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "EncounterLoader Error: Could not open file: " << filepath << std::endl;
            return std::nullopt;
        }

        json j;
        try {
            file >> j;
            EncounterData data;
            data.encounterId = j.value("encounterId", "unknown_encounter");
            data.name = j.value("name", "Unnamed Encounter");

            if (j.contains("map")) {
                const auto& jMap = j["map"];
                std::string modeStr = jMap.value("mode", "Tiled");

                if (modeStr == "SingleImage") {
                    data.mapInfo.mode = MapMode::SingleImage;
                    data.mapInfo.singleImageTextureId = jMap.value("floorTextureId", "");
                }
                else {
                    data.mapInfo.mode = MapMode::Tiled;
                    data.mapInfo.tilesetId = jMap.value("tilesetId", "");
                    if (jMap.contains("tileData")) {
                        data.mapInfo.tileData = jMap["tileData"].get<std::vector<int>>();
                    }
                }
                data.mapInfo.gridWidth = jMap.value("gridWidth", 15);
                data.mapInfo.gridHeight = jMap.value("gridHeight", 15);
                data.mapInfo.tileSize = jMap.value("tileSize", 32.0f);
                data.mapInfo.backgroundTextureId = jMap.value("backgroundTextureId", "");
            }

            auto parseSpawns = [](const json& jArray, std::vector<SpawnData>& outList) {
                if (!jArray.is_array()) return;
                for (const auto& item : jArray) {
                    SpawnData spawn;
                    spawn.id = item.value("id", "");
                    spawn.logicalX = item.value("logicalX", 0.0f);
                    spawn.logicalY = item.value("logicalY", 0.0f);
                    if (!spawn.id.empty()) outList.push_back(spawn);
                }
                };

            if (j.contains("props")) parseSpawns(j["props"], data.props);
            if (j.contains("enemies")) parseSpawns(j["enemies"], data.enemies);

            if (j.contains("deploymentZone") && j["deploymentZone"].is_array()) {
                for (const auto& pos : j["deploymentZone"]) {
                    data.deploymentZone.push_back(sf::Vector2f(pos.value("x", 0.0f), pos.value("y", 0.0f)));
                }
            }

            if (j.contains("loot") && j["loot"].is_array()) {
                data.lootItemIds = j["loot"].get<std::vector<std::string>>();
            }

            return data;
        }
        catch (const json::exception& e) {
            std::cerr << "JSON Parsing Error in " << filepath << ": " << e.what() << std::endl;
            return std::nullopt;
        }
    }
}