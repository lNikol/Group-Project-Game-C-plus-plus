#pragma once
#include "Prop.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <SFML/Graphics/Rect.hpp>

namespace RPG {

    /**
     * @brief Data template representing a single prop loaded from JSON.
     */
    struct PropDefinition {
        std::string id;
        std::string name;
        std::string spritesheetId;
        sf::IntRect textureRect;

        float colliderWidth = 20.f;
        float colliderHeight = 20.f;

        bool blocksMovement = true;
        bool blocksSight = false;
    };

    /**
     * @brief Singleton Factory managing PropDefinitions and instantiating Props.
     */
    class PropFactory {
    public:
        static PropFactory& getInstance();

        /**
         * @brief Loads and parses prop blueprints from a JSON file.
         */
        bool loadFromJSON(const std::string& filepath);

        /**
         * @brief Creates a fully initialized Prop instance.
         * @param id The string key (e.g., "stone_pillar").
         * @param logicalX The logical X coordinate to place the prop.
         * @param logicalY The logical Y coordinate to place the prop.
         * @return std::shared_ptr<Prop> or nullptr if ID not found.
         */
        std::shared_ptr<Prop> createProp(const std::string& id, float logicalX, float logicalY);

    private:
        PropFactory() = default;
        std::unordered_map<std::string, PropDefinition> m_definitions;
    };
}