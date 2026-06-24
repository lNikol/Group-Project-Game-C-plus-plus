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
     *
     * ChestWindow is injected via the constructor (non-owning pointer).
     * Create it alongside VictoryWindow in GameManager / WorldMapScene:
     *
     *   m_chestWindow = std::make_unique<ChestWindow>(iconSet, font, onClose);
     *   auto handler  = std::make_shared<GameTriggerHandler>(sceneCtrl, m_chestWindow.get());
     */
    class GameTriggerHandler : public ITriggerHandler {
        ISceneController& sceneController;

    public:
        explicit GameTriggerHandler(ISceneController& ctrl)
            : sceneController(ctrl)
        {
        }

        bool handle(const TriggerComponent& trigger, GameObject* triggerEntity) override {
            switch (trigger.action) {

            case TriggerAction::ChangeScene:
                sceneController.changeScene(trigger.targetScene);
                return true;

            case TriggerAction::OpenChest: {
                // Roll from the global pool (abilities with chestChance > 0)
                auto rolledItems = LootManager::rollChestLoot();

                // Add to shared inventory, track what actually fit
                auto& party = PartyData::getInstance();
                std::vector<std::shared_ptr<IAbility>> addedItems;

                for (auto& item : rolledItems) {
                    if (party.addItem(item)) {
                        addedItems.push_back(item);
                        std::cout << "[Chest] Added: " << item << "\n";
                    }
                    else {
                        std::cout << "[Chest] Inventory full, dropped: " << item << "\n";
                    }
                }

                ChestWindow::getInstance().setResults(addedItems);

                return true;
            }

            case TriggerAction::SaveGame:
                std::cout << "[GameTriggerHandler] SaveGame\n";
                return true;

            case TriggerAction::UpdateQuest:
                if (trigger.isCompletingQuest)
                    QuestManager::getInstance().completeQuest(trigger.questId);
                else
                    QuestManager::getInstance().startQuest(trigger.questId);
                return true;

            default:
                return false; // StartDialog — passed to DialogTriggerHandler
            }
        }
    };

}