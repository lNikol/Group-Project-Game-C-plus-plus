#pragma once
#include <unordered_map>
#include "StructureDefinition.h"
#include "core/SpritesheetManager.h"

namespace RPG {

    class AssetManager {
    private:
        /** @brief Map storing definitions for each structure type */
        std::unordered_map<StructureType, StructureDefinition> structureLibrary;

    public:
        AssetManager() = default;

        /** * @brief Initializes definitions and automatically loads required textures
         * @param sm Reference to the SpritesheetManager for texture registration
         */
        void init(SpritesheetManager& sm);

        /** @brief Returns the definition for a specific structure type */
        const StructureDefinition& getDefinition(StructureType type) const;
    };
}