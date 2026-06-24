#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <set>

namespace RPG {

    struct DialogResponse {
        std::string text;
        std::string nextNode;
    };

    struct DialogCondition {
        std::string questId;
        std::string status;
        std::string trueNode;
        std::string falseNode;
    };

    struct DialogNode {
        std::string text;
        std::vector<DialogResponse> responses;
        std::string triggerEvent = "";
        DialogCondition questCondition;
    };

    struct DialogTree {
        int id;
        std::string npcName;
        std::string npcPortrait;
        std::unordered_map<std::string, DialogNode> nodes;
    };

    class DialogManager {
    public:
        static DialogManager& getInstance();

        bool loadFromJSON(const std::string& filepath);

        void startDialog(int id);
        void endDialog();
        void selectResponse(int index);

        bool isActive() const;
        const DialogTree* getCurrentDialog() const;
        const DialogNode* getCurrentNode() const;
        const DialogTree* getCurrentTree() const;
        
        bool isExhausted(int dialogId) const;
        void markExhausted(int dialogId);

    private:
        DialogManager();

        void processCurrentNode();
        
        std::unordered_map<int, DialogTree> m_dialogs;
        
        int m_currentDialogId = -1;
        std::string m_currentNodeId = "";
        
        std::set<int> m_exhaustedDialogs;
    };

}
