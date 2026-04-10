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
        Detail,    // Flowers
        Object,    // Chests
        Structure  // Walls
    };

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
    };
}