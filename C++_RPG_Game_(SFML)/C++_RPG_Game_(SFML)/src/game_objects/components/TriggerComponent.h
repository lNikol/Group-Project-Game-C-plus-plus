#pragma once
#include "game_objects/components/Component.h"
#include "combat/EncounterLoader.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
namespace RPG {

    enum class TriggerShape {
        Circle, // radius — interaction spheres, NPC, etc.
        Rect    // square — doors, chests, entrances
    };

    enum class TriggerAction {
        ChangeScene,  
        OpenChest,    
        StartDialog,  
        SaveGame,
        UpdateQuest
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

        FactionID targetScene = FactionID::MainWorld; 
        int dialogId = -1; // for StartDialog
        std::string questId = ""; // for UpdateQuest
        bool isCompletingQuest = false; // false = Start Quest, true = Complete Quest
        
        bool oneShot = true;  // true = Activates once and disactivates
        bool active  = true;  // false = Trigger is offed
    };

}
