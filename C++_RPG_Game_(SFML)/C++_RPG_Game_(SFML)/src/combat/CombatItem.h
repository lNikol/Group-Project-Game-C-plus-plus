#pragma once
#include "CombatAbility.h"
#include "BattleManager.h"

namespace RPG {
    /**
     * @brief A data-driven item that behaves like a CombatAbility but interacts with the UI.
     * Inherits complex effect logic from CombatAbility.
     */
    class CombatItem : public CombatAbility {
    public:
        using CombatAbility::CombatAbility;

        /**
         * @brief Closes UI windows before starting the targeting process.
         */
        void execute() override {
            if (!canBeCast()) return;
            auto activeUnit = m_manager->getActiveUnit();
            if (!activeUnit) return;

            setOwner(activeUnit.get());

            // 1. Close all UI windows through the manager/HUD
            m_manager->getHUD()->closeAllWindows();

            // 2. Start standard targeting procedure inherited from CombatAbility
            m_manager->startTargeting(this);
        }
    };
}