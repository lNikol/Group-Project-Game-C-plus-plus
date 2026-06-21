#pragma once
#include "game_objects/components/ITriggerHandler.h"
#include "core/DialogManager.h"
#include <iostream>

namespace RPG {

    class DialogTriggerHandler : public ITriggerHandler {
    public:
        bool handle(const TriggerComponent& trigger, GameObject* triggerEntity) override {
            if (trigger.action != TriggerAction::StartDialog) return false;

            DialogManager::getInstance().startDialog(trigger.dialogId);
            return true;
        }
    };

}
