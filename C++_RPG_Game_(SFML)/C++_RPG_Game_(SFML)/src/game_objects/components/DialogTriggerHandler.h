#pragma once
#include "game_objects/components/ITriggerHandler.h"
#include <iostream>

namespace RPG {

    /**
      * @brief Stub for FO responsible for implementing dialogs.
      *
      * FO only needs to replace the handle() implementation —
      * the rest of the system requires no changes.
      *
      * The only steps required:
      *
      * 1. Get dialogId from trigger.dialogId
      * 2. Load the dialog from a file/database
      * 3. Open the dialog UI
      * 4. Return true
      */
    class DialogTriggerHandler : public ITriggerHandler {
    public:
        bool handle(const TriggerComponent& trigger, GameObject* triggerEntity) override {
            if (trigger.action != TriggerAction::StartDialog) return false;

            // TODO dla FO:
            // dialogManager.startDialog(trigger.dialogId);
            std::cout << "[DialogTriggerHandler] StartDialog: dialogId="
                      << trigger.dialogId << " - TODO\n";
            return true;
        }
    };

}
