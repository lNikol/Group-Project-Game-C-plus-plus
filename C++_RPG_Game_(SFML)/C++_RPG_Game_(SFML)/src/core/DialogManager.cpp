#include "DialogManager.h"
#include "EventBus.h"
#include "QuestManager.h"
#include <fstream>
#include <iostream>
#include <external/json.hpp>

using json = nlohmann::json;

namespace RPG {

    DialogManager::DialogManager() {}

    DialogManager& DialogManager::getInstance() {
        static DialogManager instance;
        return instance;
    }

    bool DialogManager::loadFromJSON(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "CRITICAL: Failed to open dialogs file: " << filepath << std::endl;
            return false;
        }

        json j;
        try {
            file >> j;
        }
        catch (json::parse_error& e) {
            std::cerr << "JSON Parse Error in Dialogs: " << e.what() << std::endl;
            return false;
        }

        if (!j.contains("dialogs") || !j["dialogs"].is_object()) {
            std::cerr << "JSON Error: 'dialogs' object missing." << std::endl;
            return false;
        }

        for (auto& [key, val] : j["dialogs"].items()) {
            DialogTree tree;
            try {
                tree.id = std::stoi(key);
            } catch (...) {
                continue;
            }
            tree.npcName = val.value("npcName", "Unknown");

            if (val.contains("nodes") && val["nodes"].is_object()) {
                for (auto& [nodeKey, nodeVal] : val["nodes"].items()) {
                    DialogNode node;
                    node.text = nodeVal.value("text", "");
                    node.triggerEvent = nodeVal.value("triggerEvent", "");

                    if (nodeVal.contains("questCondition") && nodeVal["questCondition"].is_object()) {
                        auto& qc = nodeVal["questCondition"];
                        node.questCondition.questId = qc.value("questId", "");
                        node.questCondition.status = qc.value("status", "Completed");
                        node.questCondition.trueNode = qc.value("trueNode", "");
                        node.questCondition.falseNode = qc.value("falseNode", "");
                    }

                    if (nodeVal.contains("responses") && nodeVal["responses"].is_array()) {
                        for (auto& respVal : nodeVal["responses"]) {
                            DialogResponse resp;
                            resp.text = respVal.value("text", "");
                            resp.nextNode = respVal.value("nextNode", "exit");
                            node.responses.push_back(resp);
                        }
                    }
                    tree.nodes[nodeKey] = node;
                }
            }
            m_dialogs[tree.id] = tree;
            std::cout << "[DialogManager] Loaded Dialog ID: " << tree.id << std::endl;
        }
        return true;
    }

    void DialogManager::startDialog(int id) {
        if (m_dialogs.find(id) != m_dialogs.end()) {
            m_currentDialogId = id;
            m_currentNodeId = "start"; // default entry point
            std::cout << "[DialogManager] Started Dialog " << id << std::endl;
            
            processCurrentNode();
        } else {
            std::cerr << "[DialogManager] Warning: Unknown dialog id " << id << std::endl;
        }
    }

    void DialogManager::endDialog() {
        m_currentDialogId = -1;
        m_currentNodeId = "";
        std::cout << "[DialogManager] Dialog ended." << std::endl;
    }

    void DialogManager::processCurrentNode() {
        auto* node = getCurrentNode();
        if (!node) return;

        // Check if there is a quest condition
        if (!node->questCondition.questId.empty()) {
            QuestStatus currentStatus = QuestManager::getInstance().getQuestStatus(node->questCondition.questId);
            QuestStatus requiredStatus = QuestStatus::NotStarted;
            if (node->questCondition.status == "Active") requiredStatus = QuestStatus::Active;
            else if (node->questCondition.status == "ReadyToTurnIn") requiredStatus = QuestStatus::ReadyToTurnIn;
            else if (node->questCondition.status == "Completed") requiredStatus = QuestStatus::Completed;

            if (currentStatus == requiredStatus) {
                m_currentNodeId = node->questCondition.trueNode;
            } else {
                m_currentNodeId = node->questCondition.falseNode;
            }
            processCurrentNode(); // recursive check
            return;
        }

        // Process trigger event when finally settling on a visible node
        if (!node->triggerEvent.empty()) {
            std::cout << "[DialogManager] Triggered Event: " << node->triggerEvent << std::endl;
            if (node->triggerEvent.rfind("StartQuest_", 0) == 0) {
                std::string questId = node->triggerEvent.substr(11);
                QuestManager::getInstance().startQuest(questId);
            } else if (node->triggerEvent.rfind("CompleteQuest_", 0) == 0) {
                std::string questId = node->triggerEvent.substr(14);
                QuestManager::getInstance().completeQuest(questId);
            } else if (node->triggerEvent.rfind("Objective_", 0) == 0) {
                std::string objectiveId = node->triggerEvent.substr(10);
                EventBus::getInstance().publish(DialogObjectiveEvent(objectiveId));
            }
        }
    }

    void DialogManager::selectResponse(int index) {
        if (!isActive()) return;
        auto* node = getCurrentNode();
        if (!node) return;
        
        if (index >= 0 && index < node->responses.size()) {
            const auto& resp = node->responses[index];
            m_currentNodeId = resp.nextNode;
            
            if (m_currentNodeId == "exit" || m_currentNodeId == "") {
                endDialog();
            } else {
                processCurrentNode();
            }
        }
    }

    bool DialogManager::isActive() const {
        return m_currentDialogId != -1;
    }

    const DialogTree* DialogManager::getCurrentDialog() const {
        if (isActive() && m_dialogs.find(m_currentDialogId) != m_dialogs.end()) {
            return &m_dialogs.at(m_currentDialogId);
        }
        return nullptr;
    }

    const DialogNode* DialogManager::getCurrentNode() const {
        auto* tree = getCurrentDialog();
        if (tree && tree->nodes.find(m_currentNodeId) != tree->nodes.end()) {
            return &tree->nodes.at(m_currentNodeId);
        }
        return nullptr;
    }
}
