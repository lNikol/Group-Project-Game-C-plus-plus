#include "QuestGiverComponent.h"

namespace RPG {

    QuestGiverComponent::QuestGiverComponent(const std::string& startQuestId, const std::string& completeQuestId)
        : questToStart(startQuestId), questToComplete(completeQuestId)
    {
    }

    const std::string& QuestGiverComponent::getQuestToStart() const {
        return questToStart;
    }

    const std::string& QuestGiverComponent::getQuestToComplete() const {
        return questToComplete;
    }

}
