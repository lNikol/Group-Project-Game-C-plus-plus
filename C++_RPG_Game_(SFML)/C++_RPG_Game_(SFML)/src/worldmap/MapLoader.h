#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <algorithm>
#include "worldmap/WorldMap.h"
#include "worldmap/StructureDefinition.h"
#include "game_objects/components/TriggerComponent.h"
#include "core/Constants.h"

namespace RPG {

    /**
     * @brief Parses Tiled files (.tmj) and populates the WorldMap.
     *
     * MapLoader is the single source of truth that:
     * 1. Reads JSON from Tiled.
     * 2. Creates entities via Factory.
     * 3. Queries ColliderComponent to calculate hitbox ranges.
     * 4. Sets tile.blocksMovement / blocksPlacement flags.
     *
     */
    class MapLoader {
    public:
        static bool loadFromTiled(const std::string& path, WorldMap& map);

        /**
          * @brief Creates a structure entity, sets tilemap flags, and adds it to the map.
          *
          * Moved from WorldMap::placeStructure().
          * MapLoader is aware of ColliderComponent because it is responsible for
          * building entities. After this operation, WorldMap only sees the
          * updated flags on the tiles.
          */

        static bool placeStructure(WorldMap& map, float worldX, float worldY,
               const std::string& id, uint8_t zIndex);

    private:
        static void processTileLayer(const nlohmann::json& layer, WorldMap& map);
        static void processObjectLayer(const nlohmann::json& layer, WorldMap& map);
        static FactionID mapFaction(std::string name);
        static TriggerAction mapTrigger(const std::string& str);
        static std::string getStrProperty(const nlohmann::json& el, const std::string& name, const std::string& def);
        static int getIntProperty(const nlohmann::json& el, const std::string& name, int def);
        static bool getBoolProperty(const nlohmann::json& el, const std::string& name, bool def);
        static uint8_t determineZIndex(const nlohmann::json& layer);

        static int32_t offsetX;
        static int32_t offsetY;
    };

}
