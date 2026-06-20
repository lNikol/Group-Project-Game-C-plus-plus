#pragma once
#include "game_objects/components/Component.h"
#include <string>

namespace RPG {

    class QuestGiverComponent : public Component {
        std::string questToStart;
        std::string questToComplete;

    public:
        QuestGiverComponent(const std::string& startQuestId = "", const std::string& completeQuestId = "");

        const std::string& getQuestToStart() const;
        const std::string& getQuestToComplete() const;
    };

}
