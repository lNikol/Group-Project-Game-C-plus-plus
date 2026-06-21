#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace RPG {

    struct DialogResponse {
        std::string text;
        std::string nextNode;
    };

    struct DialogNode {
        std::string text;
        std::vector<DialogResponse> responses;
        std::string triggerEvent = "";
    };

    struct DialogTree {
        int id;
        std::string npcName;
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

    private:
        DialogManager();
        
        std::unordered_map<int, DialogTree> m_dialogs;
        
        int m_currentDialogId = -1;
        std::string m_currentNodeId = "";
    };

}
