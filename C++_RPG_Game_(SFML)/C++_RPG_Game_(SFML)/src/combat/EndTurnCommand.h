#pragma once
#include "Interfaces/IAbility.h"
#include <functional>
#include <iostream>

namespace RPG {

    /**
     * @brief A special ability that ends the current unit's turn.
     * * This is placed in the hotbar like a normal skill.
     */
    class EndTurnCommand : public IAbility {
    public:
        /**
         * @param callback The function to run when executed (usually BattleManager::endTurn).
         */
        EndTurnCommand(std::function<void()> callback)
            : m_callback(callback)
        {
        }

        // ==============================
        // Visuals
        // ==============================
        int getIconIndex() const override { return 6; }

        std::string getTooltip() const override { return "End Turn"; }

        // ==============================
        // Mechanics
        // ==============================
        bool canBeCast() const override { return true; } // Always usable
        int getCharges() const override { return -1; }   // Infinite
        int getCooldown() const override { return 0; }   // No cooldown

        // ==============================
        // Execution
        // ==============================
        void execute() override {
            if (m_callback) {
                std::cout << "Ending Turn..." << std::endl;
                m_callback();
            }
        }

    private:
        std::function<void()> m_callback;
    };
}