#pragma once
#include <string>
#include <vector>
#include <optional>
#include <SFML/System/Vector2.hpp>
#include "combat/CombatMap.h"

namespace RPG {

    struct EncounterMapData {
        MapMode mode = MapMode::Tiled;
        int gridWidth = 15;
        int gridHeight = 15;
        float tileSize = 32.0f;
        std::string tilesetId;
        std::string backgroundTextureId;
        std::string singleImageTextureId;
        std::vector<int> tileData;
    };

    struct SpawnData {
        std::string id;
        float logicalX = 0.0f;
        float logicalY = 0.0f;
    };

    struct EncounterData {
        std::string encounterId;
        std::string name;

        EncounterMapData mapInfo;
        std::vector<SpawnData> props;
        std::vector<SpawnData> enemies;
        std::vector<sf::Vector2f> deploymentZone;
        std::vector<std::string> lootItemIds;
    };

    class EncounterLoader {
    public:
        static std::optional<EncounterData> loadFromFile(const std::string& filepath);
    };
}