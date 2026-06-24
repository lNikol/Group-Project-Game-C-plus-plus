#pragma once
#include <functional>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace RPG {
    enum class EventType {
        EnemyDefeated,
        ItemCollected,
        LocationReached,
        QuestCompleted,
        QuestStarted,
        QuestReadyToTurnIn,
        DialogObjective,
        TriggerBattle
    };

    struct GameEvent {
        virtual ~GameEvent() = default;
        virtual EventType getType() const = 0;
    };

    struct EnemyDefeatedEvent : public GameEvent {
        std::string enemyId;
        EnemyDefeatedEvent(const std::string& id) : enemyId(id) {}
        EventType getType() const override { return EventType::EnemyDefeated; }
    };

    struct ItemCollectedEvent : public GameEvent {
        std::string itemId;
        int amount;
        ItemCollectedEvent(const std::string& id, int amt) : itemId(id), amount(amt) {}
        EventType getType() const override { return EventType::ItemCollected; }
    };

    struct QuestCompletedEvent : public GameEvent {
        std::string questId;
        int xpReward;
        int goldReward;
        QuestCompletedEvent(const std::string& id, int xp, int gold) 
            : questId(id), xpReward(xp), goldReward(gold) {}
        EventType getType() const override { return EventType::QuestCompleted; }
    };

    struct QuestStartedEvent : public GameEvent {
        std::string questId;
        QuestStartedEvent(const std::string& id) : questId(id) {}
        EventType getType() const override { return EventType::QuestStarted; }
    };

    struct QuestReadyToTurnInEvent : public GameEvent {
        std::string questId;
        QuestReadyToTurnInEvent(const std::string& id) : questId(id) {}
        EventType getType() const override { return EventType::QuestReadyToTurnIn; }
    };

    struct DialogObjectiveEvent : public GameEvent {
        std::string objectiveId;
        DialogObjectiveEvent(const std::string& id) : objectiveId(id) {}
        EventType getType() const override { return EventType::DialogObjective; }
    };

    struct TriggerBattleEvent : public GameEvent {
        std::string encounterFile;
        TriggerBattleEvent(const std::string& file) : encounterFile(file) {}
        EventType getType() const override { return EventType::TriggerBattle; }
    };

    class EventBus {
    public:
        using EventCallback = std::function<void(const GameEvent&)>;

        static EventBus& getInstance();

        void subscribe(EventType type, EventCallback callback);
        void publish(const GameEvent& event);

    private:
        EventBus() = default;
        std::unordered_map<EventType, std::vector<EventCallback>> m_subscribers;
    };
}
