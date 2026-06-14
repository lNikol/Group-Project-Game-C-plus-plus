#pragma once
#include "game_objects/components/TriggerComponent.h"

namespace RPG {

    class GameObject;

    /**
     * @brief Interface for handling triggers.
     *
     * TriggerSystem does not know what a handler does — it only calls handle()
     * and leaves the implementation details to the handler.
     *
     * Handlers are passed to TriggerSystem through the constructor.
     * If a handler does not support a given action, it returns false,
     * and TriggerSystem tries the next one.
     *
     * Example usage:
     * systemManager.addSystem(
     *     *worldMap,
     *     std::make_shared<GameTriggerHandler>(sceneController),
     *     std::make_shared<DialogTriggerHandler>(dialogManager)
     * );
     */
    class ITriggerHandler {
    public:
        virtual ~ITriggerHandler() = default;

        /**
        * @brief Handles a trigger action.
        * @param trigger Trigger component containing data (action, dialogId, etc.)
        * @param triggerEntity Trigger entity (position and other components)
        * @return true if the handler processed the action,
        * false if the action is not handled by this handler
        */
        virtual bool handle(const TriggerComponent& trigger, GameObject* triggerEntity) = 0;
    };

}
