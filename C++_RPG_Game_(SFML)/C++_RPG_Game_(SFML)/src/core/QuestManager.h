#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace RPG {

    enum class QuestStatus {
        NotStarted,
        Active,
        ReadyToTurnIn,
        Completed
    };

    struct QuestObjective {
        std::string id;
        std::string description;
        std::string type; // e.g. "KILL_ENEMY", "COLLECT_ITEM"
        std::string targetId;
        int requiredAmount = 1;
        int currentAmount = 0;
        bool isCompleted = false;
    };

    struct QuestReward {
        int xp = 0;
        int gold = 0;
    };

    struct Quest {
        std::string id;
        std::string title;
        std::string description;
        QuestStatus status = QuestStatus::NotStarted;
        std::string nextQuestId = "";
        std::vector<QuestObjective> objectives;
        QuestReward rewards;
    };

    class QuestManager {
    public:
        static QuestManager& getInstance();

        bool loadFromJSON(const std::string& filepath);
        
        void startQuest(const std::string& id);
        bool completeQuest(const std::string& id);
        
        QuestStatus getQuestStatus(const std::string& id) const;
        
        std::vector<const Quest*> getActiveQuests() const;
        std::vector<const Quest*> getCompletedQuests() const;
        
        const Quest* getQuest(const std::string& id) const;

    private:
        QuestManager(); // private constructor for singleton
        
        void handleEvent(struct GameEvent const& event);
        
        std::unordered_map<std::string, Quest> m_quests;
    };

}
