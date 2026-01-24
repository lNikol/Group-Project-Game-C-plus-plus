#pragma once
#include "Interfaces/IAbility.h"
#include "BattleManager.h"

namespace RPG {

    class MoveCommand : public IAbility {
    public:
        MoveCommand(BattleManager& manager) : m_manager(manager) {}

        int getIconIndex() const override { return 12; }
        std::string getTooltip() const override { return "Move (Costs Stamina)"; }

        bool canBeCast() const override { return true; }
        int getCharges() const override { return -1; }
        int getCooldown() const override { return 0; }

        void execute() override {
            // Tell manager to start moving
            m_manager.startMovementMode();
        }

    private:
        BattleManager& m_manager;
    };
}