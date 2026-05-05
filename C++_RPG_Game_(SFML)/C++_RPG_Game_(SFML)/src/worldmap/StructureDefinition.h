#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <cstdint>

namespace RPG {

    /** @brief Defines the visual and physical properties of a map structure */
    enum class StructureType {
        None,
        InvisibleBlock, // To block the neighborhood of the center point of a large object
        Rock, Tree, Wall,           // Obstacles
        Water, River, Swamp, Grass, Sand, // Ground types
        Decoration,
        interactObj,
        Camp, SaveZone, FactionBase, House // Special zones
    };

    enum class PlacementLayer {
        Ground,    // Ground
        Decoration,    // Flowers
        Object,    // Chests
        Structure  // Walls
    };

    namespace LayerZIndex {
        enum class ZLayer : uint8_t {
            Ground = 0,
            BelowPlayer = 1,
            Main = 2,
            AbovePlayer = 3,
            UI = 4
        };
    }

    struct StructureMetadata {
        float maxHealth = 0.f;
        bool isDestructible = false;
        bool blocksPlacement = false;
        bool blocksMovement = false;
        bool allowsRegeneration = false;
        bool canSpawnMob = false;
        float speedModifier = 1.f;
    };

    class StructureDefinition {
    public:
        std::string name;
        StructureType type;
        PlacementLayer layer;
        std::string textureKey;
        sf::Vector2u textureStartPos;
        sf::Vector2u size;
        sf::Vector2f hitboxSize;
        sf::Vector2f hitboxOffset;
        struct StructureMetadata meta;
        uint8_t defaultZIndex = static_cast<uint8_t>(LayerZIndex::ZLayer::Main);
    };
}