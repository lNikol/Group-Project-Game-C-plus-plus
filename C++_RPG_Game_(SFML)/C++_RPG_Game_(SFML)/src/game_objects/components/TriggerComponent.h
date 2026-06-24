#pragma once
#include "game_objects/components/Component.h"
#include <SFML/Graphics.hpp>
#include <string>

namespace RPG {

    enum class TriggerShape {
        Circle, // radius — interation spheres, NPC, etc.
        Rect    // square — doors, chests, entrances
    };

    enum class TriggerAction {
        ChangeScene,  
        OpenChest,    
        StartDialog,  
        SaveGame,
        UpdateQuest,
        StartBattle
    };

    /**
      * @brief Trigger data — area shape and action to execute.
      *
      * A trigger is an invisible area on the map. When the player enters it,
      * TriggerSystem executes the corresponding ITriggerHandler.
      *
      * Shapes:
      *   Circle — checks the player's distance from the trigger center
      *   Rect   — checks whether the player is inside the rectangle
      *
      * oneShot = true  -> the trigger is disabled after the first activation
      * oneShot = false -> activates every time the player is inside the area
     */
    class TriggerComponent : public Component {
    public:
        TriggerShape shape = TriggerShape::Circle;
        float radius = 32.f;         
        sf::Vector2f rectSize = { 32.f, 32.f }; 

        TriggerAction action = TriggerAction::ChangeScene;

        FactionID targetScene = FactionID::NeutralOrder; 
        int dialogId = -1; // for StartDialog
        int chestId = -1; // for OpenChest
        std::string questId = ""; // for UpdateQuest
        std::string encounterFile = ""; // for StartBattle
        std::string requiredQuestId = ""; // condition to activate trigger
        bool isCompletingQuest = false; // false = Start Quest, true = Complete Quest

        bool oneShot = true;  // true = Activates once and disactivates
        bool active  = true;  // false = Trigger is offed
    };

}
