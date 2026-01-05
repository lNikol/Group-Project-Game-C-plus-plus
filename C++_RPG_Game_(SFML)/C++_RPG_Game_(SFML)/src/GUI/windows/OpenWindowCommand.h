#pragma once
#include "../../Interfaces/IAbility.h"
#include <functional>
#include <string>

namespace RPG {

    class OpenWindowCommand : public IAbility {
    public:
        OpenWindowCommand(int iconIndex, std::string tooltip, std::function<void()> onExecute)
            : m_iconIndex(iconIndex), m_tooltip(tooltip), m_onExecute(onExecute) {
        }

        // Visuals
        int getIconIndex() const override { return m_iconIndex; }
        std::string getTooltip() const override { return m_tooltip; }

        // Mechanics
        bool canBeCast() const override { return m_enabled; }

        int getCharges() const override { return -1; }
        int getCooldown() const override { return 0; }

        // Execution
        void execute() override {
            if (m_onExecute) {
                m_onExecute();
            }
        }

        // Helper
        void setEnabled(bool enabled) { m_enabled = enabled; }

    private:
        int m_iconIndex;
        std::string m_tooltip;
        std::function<void()> m_onExecute;
        bool m_enabled = true;
    };
}