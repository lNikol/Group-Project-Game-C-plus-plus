#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <cstdint>
#include "worldmap/enums.h"
#include "worldmap/AssetManager.h"
#include "AnimatedEntity.h"

namespace RPG {

    /**
     * @class NPC
     * @brief Non-player character that uses textures registered in the SpritesheetManager.
     */
    class NPC : public AnimatedEntity {
    private:
        std::string name;    // The key from the manager (e.g., "camp")
        uint8_t tileIndex;          // Index of the tile within the sheet (starting from 0)
        float interactionRadius;
        bool isFactionLeader;
        FactionID targetFaction; // The destination faction if this NPC is a leader

    public:
        /**
         * @param pos Starting position in world coordinates (pixels).
         * @param texKey Manager key (e.g., filename without .png extension).
         * @param tileIdx Tile index within the spritesheet.
         * @param npcName Name used for logging and interaction.
         */
        NPC(uint8_t tileIdx, std::string npcName, bool isLeader = false, FactionID target = FactionID::MainWorld);

        /**
         * @brief Renders the NPC using a texture retrieved from the manager.
         * @param window Reference to the render window.
         * @param sm Reference to the SpritesheetManager.
         */
        void draw(sf::RenderWindow& window, const AssetManager& sm);

        //sf::Vector2f getPosition() const;
        float getInteractionRadius() const;
        const std::string& getName() const;
        bool getIsFactionLeader() const;
        FactionID getTargetFaction() const;

        //void setPosition(sf::Vector2f pos);
    };
}