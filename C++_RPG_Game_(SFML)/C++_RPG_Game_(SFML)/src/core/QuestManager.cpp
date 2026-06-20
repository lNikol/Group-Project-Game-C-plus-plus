#include "QuestManager.h"
#include "EventBus.h"
#include <fstream>
#include <iostream>
#include <external/json.hpp> 

using json = nlohmann::json;

namespace RPG {

    QuestManager::QuestManager() {
        EventBus::getInstance().subscribe(EventType::EnemyDefeated, [this](const GameEvent& e) { this->handleEvent(e); });
        EventBus::getInstance().subscribe(EventType::ItemCollected, [this](const GameEvent& e) { this->handleEvent(e); });
    }

    QuestManager& QuestManager::getInstance() {
        static QuestManager instance;
        return instance;
    }

    bool QuestManager::loadFromJSON(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "CRITICAL: Failed to open quests file: " << filepath << std::endl;
            return false;
        }

        json j;
        try {
            file >> j;
        }
        catch (json::parse_error& e) {
            std::cerr << "JSON Parse Error: " << e.what() << std::endl;
            return false;
        }

        if (!j.contains("quests") || !j["quests"].is_array()) {
            std::cerr << "JSON Error: 'quests' array missing." << std::endl;
            return false;
        }

        for (const auto& item : j["quests"]) {
            Quest q;
            q.id = item.value("id", "unknown");
            q.title = item.value("title", "Unnamed Quest");
            q.description = item.value("description", "");
            q.status = QuestStatus::NotStarted;

            if (item.contains("objectives") && item["objectives"].is_array()) {
                for (const auto& objJson : item["objectives"]) {
                    QuestObjective obj;
                    obj.id = objJson.value("id", "unknown_obj");
                    obj.description = objJson.value("description", "");
                    obj.type = objJson.value("type", "");
                    obj.targetId = objJson.value("targetId", "");
                    obj.requiredAmount = objJson.value("requiredAmount", 1);
                    obj.currentAmount = 0;
                    obj.isCompleted = false;
                    q.objectives.push_back(obj);
                }
            }

            if (item.contains("rewards") && item["rewards"].is_object()) {
                q.rewards.xp = item["rewards"].value("xp", 0);
                q.rewards.gold = item["rewards"].value("gold", 0);
            }

            m_quests[q.id] = q;
            std::cout << "[QuestManager] Loaded Quest: " << q.title << " (" << q.id << ")" << std::endl;
        }

        return true;
    }

    void QuestManager::startQuest(const std::string& id) {
        if (m_quests.find(id) != m_quests.end()) {
            if (m_quests[id].status == QuestStatus::NotStarted) {
                m_quests[id].status = QuestStatus::Active;
                std::cout << "[QuestManager] Quest Started: " << m_quests[id].title << std::endl;
                EventBus::getInstance().publish(QuestStartedEvent(id));

                // If it has no objectives, it might instantly be ready
                if (m_quests[id].objectives.empty()) {
                    m_quests[id].status = QuestStatus::ReadyToTurnIn;
                    EventBus::getInstance().publish(QuestReadyToTurnInEvent(id));
                }
            }
        } else {
            std::cerr << "[QuestManager] Warning: Attempted to start unknown quest: " << id << std::endl;
        }
    }

    bool QuestManager::completeQuest(const std::string& id) {
        if (m_quests.find(id) != m_quests.end()) {
            if (m_quests[id].status == QuestStatus::ReadyToTurnIn) {
                m_quests[id].status = QuestStatus::Completed;
                std::cout << "[QuestManager] Quest Completed: " << m_quests[id].title << std::endl;
                
                // Publish Reward Event
                EventBus::getInstance().publish(QuestCompletedEvent(
                    id, 
                    m_quests[id].rewards.xp, 
                    m_quests[id].rewards.gold
                ));
                return true;
            } else if (m_quests[id].status == QuestStatus::Completed) {
                // already completed
                return false;
            } else {
                std::cout << "[QuestManager] Quest " << m_quests[id].title << " is not ready to turn in yet!" << std::endl;
                return false;
            }
        } else {
            std::cerr << "[QuestManager] Quest " << id << " not found!" << std::endl;
            return false;
        }
    }

    QuestStatus QuestManager::getQuestStatus(const std::string& id) const {
        auto it = m_quests.find(id);
        if (it != m_quests.end()) {
            return it->second.status;
        }
        return QuestStatus::NotStarted;
    }

    std::vector<const Quest*> QuestManager::getActiveQuests() const {
        std::vector<const Quest*> activeQuests;
        for (const auto& pair : m_quests) {
            if (pair.second.status == QuestStatus::Active || pair.second.status == QuestStatus::ReadyToTurnIn) {
                activeQuests.push_back(&pair.second);
            }
        }
        return activeQuests;
    }

    std::vector<const Quest*> QuestManager::getCompletedQuests() const {
        std::vector<const Quest*> completedQuests;
        for (const auto& pair : m_quests) {
            if (pair.second.status == QuestStatus::Completed) {
                completedQuests.push_back(&pair.second);
            }
        }
        return completedQuests;
    }

    const Quest* QuestManager::getQuest(const std::string& id) const {
        auto it = m_quests.find(id);
        if (it != m_quests.end()) {
            return &it->second;
        }
        return nullptr;
    }

    void QuestManager::handleEvent(const GameEvent& event) {
        for (auto& pair : m_quests) {
            Quest& q = pair.second;
            if (q.status != QuestStatus::Active) continue;
            
            bool allCompleted = true;
            for (auto& obj : q.objectives) {
                if (obj.isCompleted) continue;
                
                if (event.getType() == EventType::EnemyDefeated && obj.type == "KILL_ENEMY") {
                    const auto& enemyEvent = static_cast<const EnemyDefeatedEvent&>(event);
                    if (obj.targetId == enemyEvent.enemyId) {
                        obj.currentAmount++;
                        if (obj.currentAmount >= obj.requiredAmount) {
                            obj.currentAmount = obj.requiredAmount;
                            obj.isCompleted = true;
                        }
                    }
                }
                
                if (event.getType() == EventType::ItemCollected && obj.type == "COLLECT_ITEM") {
                    const auto& itemEvent = static_cast<const ItemCollectedEvent&>(event);
                    if (obj.targetId == itemEvent.itemId) {
                        obj.currentAmount += itemEvent.amount;
                        if (obj.currentAmount >= obj.requiredAmount) {
                            obj.currentAmount = obj.requiredAmount;
                            obj.isCompleted = true;
                        }
                    }
                }
                
                if (!obj.isCompleted) allCompleted = false;
            }
            
            if (allCompleted && !q.objectives.empty()) {
                q.status = QuestStatus::ReadyToTurnIn;
                std::cout << "[QuestManager] Quest Ready to Turn In: " << q.title << std::endl;
                EventBus::getInstance().publish(QuestReadyToTurnInEvent(q.id));
            }
        }
    }

}
