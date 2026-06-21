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
            
            auto* node = getCurrentNode();
            if (node && !node->triggerEvent.empty()) {
                std::cout << "[DialogManager] Triggered Event: " << node->triggerEvent << std::endl;
                if (node->triggerEvent.rfind("StartQuest_", 0) == 0) {
                    std::string questId = node->triggerEvent.substr(11);
                    QuestManager::getInstance().startQuest(questId);
                } else if (node->triggerEvent.rfind("CompleteQuest_", 0) == 0) {
                    std::string questId = node->triggerEvent.substr(14);
                    QuestManager::getInstance().completeQuest(questId);
                }
            }
        } else {
            std::cerr << "[DialogManager] Warning: Unknown dialog id " << id << std::endl;
        }
    }

    void DialogManager::endDialog() {
        m_currentDialogId = -1;
        m_currentNodeId = "";
        std::cout << "[DialogManager] Dialog ended." << std::endl;
    }

    void DialogManager::selectResponse(int index) {
        if (!isActive()) return;
        auto* node = getCurrentNode();
        if (!node) return;

        // Process trigger on the current node BEFORE advancing, or after?
        // Actually, trigger should happen when entering the node or when picking a response?
        // Usually, triggerEvent is tied to the node itself when it's displayed, or to the response.
        // Let's trigger it when the node is displayed, so we'll do it in startDialog and selectResponse after changing the node.
        
        if (index >= 0 && index < node->responses.size()) {
            const auto& resp = node->responses[index];
            m_currentNodeId = resp.nextNode;
            
            auto* next_node = getCurrentNode();
            if (next_node && !next_node->triggerEvent.empty()) {
                std::cout << "[DialogManager] Triggered Event: " << next_node->triggerEvent << std::endl;
                if (next_node->triggerEvent.rfind("StartQuest_", 0) == 0) {
                    std::string questId = next_node->triggerEvent.substr(11);
                    QuestManager::getInstance().startQuest(questId);
                } else if (next_node->triggerEvent.rfind("CompleteQuest_", 0) == 0) {
                    std::string questId = next_node->triggerEvent.substr(14);
                    QuestManager::getInstance().completeQuest(questId);
                }
            }

            if (m_currentNodeId == "exit" || m_currentNodeId == "") {
                endDialog();
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
