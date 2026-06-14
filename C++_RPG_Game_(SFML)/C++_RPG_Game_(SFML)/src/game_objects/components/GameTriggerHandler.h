#pragma once
#include "game_objects/components/ITriggerHandler.h"
#include "scenes/ISceneController.h"
#include "worldmap/enums.h"
#include <iostream>

namespace RPG {

    /**
     * @brief Handles game actions: scene changes, chests, saving, etc.
     */
    class GameTriggerHandler : public ITriggerHandler {
        ISceneController& sceneController;

    public:
        explicit GameTriggerHandler(ISceneController& ctrl)
            : sceneController(ctrl) {}

        bool handle(const TriggerComponent& trigger, GameObject* triggerEntity) override {
            switch (trigger.action) {

            case TriggerAction::ChangeScene:
                sceneController.changeScene(trigger.targetScene);
                return true;

            case TriggerAction::OpenChest:
                // TODO: open chests by chestId
                std::cout << "[GameTriggerHandler] OpenChest: chestId=" << trigger.chestId << "\n";
                return true;

            case TriggerAction::SaveGame:
                // TODO: save manager
                std::cout << "[GameTriggerHandler] SaveGame\n";
                return true;

            default:
                return false; // StartDialog — oddaj do DialogTriggerHandler
            }
        }
    };

}
