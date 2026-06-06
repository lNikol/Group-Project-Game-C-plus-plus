#pragma once
#include "StructureDefinition.h"
#include "core/Constants.h"
#include <cstdint>
#include <vector>

namespace RPG {

    struct Position {
        int32_t x, y, z;
    };

    /**
     * @brief Raw spatial data for a single tile.
     */
    class Tile {
    public:
        StructureType       groundType    = StructureType::Grass;
        Position            pos           = { 0, 0, 0 };
        float               speedModifier = 1.f;
        float               localDangerLvl = 0.f;
        GameConfig::Faction faction       = GameConfig::Faction::NEUTRAL;
        bool                blocksMovement  = false;
        bool                blocksPlacement = false;
        std::vector<uint32_t> baseGids;

        Tile() = default;
        Tile(StructureType ground, int32_t x, int32_t y, int32_t z)
            : groundType(ground), pos({ x, y, z }) {}

        bool isAvailableForSpawn() const { return !blocksMovement; }
        bool isAvailableForPlace() const { return !blocksPlacement; }

        bool canAccept(PlacementLayer newLayer) const {
            if (blocksPlacement) return false;
            if (newLayer == PlacementLayer::Object && blocksMovement) return false;
            return true;
        }

        void release() {
            blocksMovement  = false;
            blocksPlacement = false;
        }
    };

}
