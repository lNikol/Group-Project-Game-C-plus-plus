#pragma once
#include "game_objects/components/ITriggerHandler.h"
#include "game_objects/components/TriggerComponent.h"
#include "scenes/ISceneController.h"
#include "worldmap/enums.h"
#include "core/QuestManager.h"
#include "combat/LootManager.h"
#include "combat/PartyData.h"
#include "GUI/windows/ChestWindow.h"
#include <iostream>

namespace RPG {

    /**
     * @brief Handles game actions: scene changes, chests, saving, quests.
     */
    class GameTriggerHandler : public ITriggerHandler {
        ISceneController& sceneController;
        ChestWindow* m_chestWindow = nullptr; // non-owning

    public:
        explicit GameTriggerHandler(ISceneController& ctrl,
            ChestWindow* chestWindow = nullptr)
            : sceneController(ctrl), m_chestWindow(chestWindow) {
        }

        bool handle(const TriggerComponent& trigger, GameObject* triggerEntity) override {
            std::cout << "[TriggerHandler] caught trigger: " << static_cast<int>(trigger.action) << "\n";

            switch (trigger.action) {

            case TriggerAction::ChangeScene:
                sceneController.changeScene(trigger.targetScene);
                return true;

            case TriggerAction::OpenChest: {
                // Roll from the global chest loot pool (abilities with chestChance > 0)
                auto rolledItems = LootManager::rollChestLoot();

                std::cout << "[TriggerHandler:OpenChest] rolledItems generated: " << rolledItems.size() << "\n";

                // Add items to shared inventory, track what actually fit
                auto& party = PartyData::getInstance();
                std::vector<std::shared_ptr<IAbility>> addedItems;

                for (auto& item : rolledItems) {
                    if (party.addItem(item)) {
                        addedItems.push_back(item);
                        std::cout << "[Chest] Added to inventory: " << item << "\n";
                    }
                    else {
                        std::cout << "[Chest] Inventory full, dropped: " << item << "\n";
                    }
                }

                // Show result window
                // ChestWindow::getInstance().showResults(addedItems); - todo

                return true;
            }

            case TriggerAction::SaveGame:
                // TODO: SaveManager
                std::cout << "[GameTriggerHandler] SaveGame\n";
                return true;

            case TriggerAction::UpdateQuest:
                if (trigger.isCompletingQuest) {
                    QuestManager::getInstance().completeQuest(trigger.questId);
                } else {
                    QuestManager::getInstance().startQuest(trigger.questId);
                }
                return true;

            default:
                return false; // StartDialog — passed to DialogTriggerHandler
            }
        }
    };

}
