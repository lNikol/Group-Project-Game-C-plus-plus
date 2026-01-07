#pragma once
#include <string>
#include <cstdint>

namespace RPG {
    /** @brief Defines the visual and physical properties of a map structure */
    enum class StructureType {
        None,
        InvisibleBlock, // To block the neighborhood of the center point of a large object
        Rock, Tree, Wall,           // Obstacles
        Water, River, Swamp, Grass, Sand, // Ground types
        Camp, SaveZone, FactionBase // Special zones
    };

    /** @brief Defines the grid size of a structure (e.g., 1x1, 2x2) */
    struct StructureSize {
        uint16_t width, height;
    };

    /** @brief Data container for tile/structure properties */
    class StructureDefinition {
    public:
        std::string name;
        std::string textureKey;  // Key used to fetch texture from SpritesheetManager
        StructureSize size;      // Grid dimensions
        float speedModifier;     // Movement speed multiplier (1.0f = normal)
        bool canSpawnMob;        // Enemy spawn flag
        bool blocksMovement;     // Collision flag
        bool allowsRegeneration; // Healing/Save zone flag
    };
}